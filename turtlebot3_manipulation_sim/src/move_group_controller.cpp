#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include <rclcpp/rclcpp.hpp>

#include <moveit/move_group_interface/move_group_interface.hpp>
#include <moveit/planning_interface/planning_interface.hpp>

using namespace std::chrono_literals;

class TurtleBot3ManipulatorController
{
public:
  explicit TurtleBot3ManipulatorController(
    const rclcpp::Node::SharedPtr & node)
  : node_(node),
    arm_(node_, "arm"),
    gripper_(node_, "gripper")
  {
    arm_.setMaxVelocityScalingFactor(0.3);
    arm_.setMaxAccelerationScalingFactor(0.3);

    gripper_.setMaxVelocityScalingFactor(0.2);
    gripper_.setMaxAccelerationScalingFactor(0.2);

    RCLCPP_INFO(node_->get_logger(), "MoveGroupInterface controller started.");
    RCLCPP_INFO(
      node_->get_logger(),
      "Arm planning frame: %s",
      arm_.getPlanningFrame().c_str());
    RCLCPP_INFO(
      node_->get_logger(),
      "Arm end effector: %s",
      arm_.getEndEffectorLink().c_str());
  }

  bool moveArm(const std::vector<double> & joint_target)
  {
    if (joint_target.size() != 4) {
      RCLCPP_ERROR(
        node_->get_logger(),
        "Arm target must contain exactly 4 joint values.");
      return false;
    }

    arm_.setStartStateToCurrentState();
    arm_.setJointValueTarget(joint_target);

    moveit::planning_interface::MoveGroupInterface::Plan plan;

    RCLCPP_INFO(node_->get_logger(), "Planning arm trajectory...");

    const auto result = arm_.plan(plan);

    if (result != moveit::core::MoveItErrorCode::SUCCESS) {
      RCLCPP_ERROR(node_->get_logger(), "Arm planning failed.");
      return false;
    }

    RCLCPP_INFO(node_->get_logger(), "Arm planning succeeded. Executing...");

    const auto execution_result = arm_.execute(plan);

    if (execution_result != moveit::core::MoveItErrorCode::SUCCESS) {
      RCLCPP_ERROR(node_->get_logger(), "Arm execution failed.");
      return false;
    }

    RCLCPP_INFO(node_->get_logger(), "Arm execution succeeded.");
    return true;
  }

  bool moveGripper(double target)
  {
    gripper_.setStartStateToCurrentState();
    gripper_.setJointValueTarget("gripper_left_joint",target);

    moveit::planning_interface::MoveGroupInterface::Plan plan;

    RCLCPP_INFO(
      node_->get_logger(),
      "Planning gripper trajectory to %.4f rad...",
      target);

    const auto result = gripper_.plan(plan);

    if (result != moveit::core::MoveItErrorCode::SUCCESS) {
      RCLCPP_ERROR(node_->get_logger(), "Gripper planning failed.");
      return false;
    }

    RCLCPP_INFO(node_->get_logger(), "Executing gripper trajectory...");

    const auto execution_result = gripper_.execute(plan);

    if (execution_result != moveit::core::MoveItErrorCode::SUCCESS) {
      RCLCPP_ERROR(node_->get_logger(), "Gripper execution failed.");
      return false;
    }

    RCLCPP_INFO(node_->get_logger(), "Gripper execution succeeded.");
    return true;
  }

private:
  rclcpp::Node::SharedPtr node_;
  moveit::planning_interface::MoveGroupInterface arm_;
  moveit::planning_interface::MoveGroupInterface gripper_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<rclcpp::Node>(
    "turtlebot3_manipulation_controller",
    rclcpp::NodeOptions()
      .automatically_declare_parameters_from_overrides(true));

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);

  std::thread spinner([&executor]() {
    executor.spin();
  });

  // Give MoveGroupInterface time to connect to move_group.
  std::this_thread::sleep_for(2s);

  TurtleBot3ManipulatorController controller(node);

  // Home-ish pose.
  const bool arm_ok = controller.moveArm({
    0.0,
    -1.0,
    0.7,
    0.3
  });

  if (!arm_ok) {
    RCLCPP_ERROR(node->get_logger(), "Initial arm movement failed.");
    executor.cancel();
    spinner.join();
    rclcpp::shutdown();
    return 1;
  }

  std::this_thread::sleep_for(2s);

  // Open gripper.
  if (!controller.moveGripper(0.01)) {
    RCLCPP_ERROR(node->get_logger(), "Gripper opening failed.");
    executor.cancel();
    spinner.join();
    rclcpp::shutdown();
    return 1;
  }

  std::this_thread::sleep_for(2s);

  // Example second arm position.
  if (!controller.moveArm({
    0.7,
    0.4,
    0.4,
    0.0
  })) {
    RCLCPP_ERROR(node->get_logger(), "Second arm movement failed.");
    executor.cancel();
    spinner.join();
    rclcpp::shutdown();
    return 1;
  }

  std::this_thread::sleep_for(2s);

  // Close gripper.
  if (!controller.moveGripper(-0.01)) {
    RCLCPP_ERROR(node->get_logger(), "Gripper closing failed.");
    executor.cancel();
    spinner.join();
    rclcpp::shutdown();
    return 1;
  }

  RCLCPP_INFO(node->get_logger(), "Demo sequence completed.");

  executor.cancel();
  spinner.join();
  rclcpp::shutdown();

  return 0;
}
