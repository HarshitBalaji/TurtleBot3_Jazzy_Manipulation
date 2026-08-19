#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "control_msgs/action/follow_joint_trajectory.hpp"
#include "trajectory_msgs/msg/joint_trajectory_point.hpp"

using namespace std::chrono_literals;

class ArmController : public rclcpp::Node
{
public:
  using FollowJointTrajectory =
    control_msgs::action::FollowJointTrajectory;

  using GoalHandle =
    rclcpp_action::ClientGoalHandle<FollowJointTrajectory>;

  ArmController()
  : Node("arm_controller")
  {
    client_ =
      rclcpp_action::create_client<FollowJointTrajectory>(
        this,
        "/arm_controller/follow_joint_trajectory");

    RCLCPP_INFO(
      this->get_logger(),
      "Waiting for arm trajectory controller...");

    if (!client_->wait_for_action_server(10s)) {
      RCLCPP_ERROR(
        this->get_logger(),
        "Action server not available.");

      rclcpp::shutdown();
      return;
    }

    RCLCPP_INFO(
      this->get_logger(),
      "Arm trajectory controller connected.");

    send_goal(
      {0.7, 0.4, 0.4, 0.0},
      3.0);
  }

private:
  void send_goal(
    const std::vector<double> & positions,
    double duration)
  {
    FollowJointTrajectory::Goal goal;

    goal.trajectory.joint_names = {
      "joint1",
      "joint2",
      "joint3",
      "joint4"
    };

    trajectory_msgs::msg::JointTrajectoryPoint point;

    point.positions = positions;

    point.time_from_start.sec =
      static_cast<int32_t>(duration);

    point.time_from_start.nanosec =
      static_cast<uint32_t>(
        (duration - static_cast<int32_t>(duration)) * 1e9);

    goal.trajectory.points.push_back(point);

    RCLCPP_INFO(
      this->get_logger(),
      "Sending trajectory goal.");

    for (size_t i = 0; i < positions.size(); ++i) {
      RCLCPP_INFO(
        this->get_logger(),
        "joint%zu = %.3f rad",
        i + 1,
        positions[i]);
    }

    rclcpp_action::Client<FollowJointTrajectory>::SendGoalOptions options;

    options.goal_response_callback =
      [this](const GoalHandle::SharedPtr & goal_handle)
      {
        if (!goal_handle) {
          RCLCPP_ERROR(
            this->get_logger(),
            "Trajectory goal was rejected.");
          return;
        }

        RCLCPP_INFO(
          this->get_logger(),
          "Trajectory goal accepted.");
      };

    options.result_callback =
      [this](const GoalHandle::WrappedResult & result)
      {
        if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
          RCLCPP_INFO(
            this->get_logger(),
            "Trajectory completed successfully.");
        } else {
          RCLCPP_ERROR(
            this->get_logger(),
            "Trajectory failed.");
        }

        rclcpp::shutdown();
      };

    client_->async_send_goal(goal, options);
  }

  rclcpp_action::Client<FollowJointTrajectory>::SharedPtr client_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<ArmController>();

  rclcpp::spin(node);

  rclcpp::shutdown();

  return 0;
}

