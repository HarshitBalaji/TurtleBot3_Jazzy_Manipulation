cmake_minimum_required(VERSION 3.8)
project(turtlebot3_manipulation_sim)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(control_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)
find_package(xacro REQUIRED)
find_package(robot_state_publisher REQUIRED)
find_package(controller_manager REQUIRED)
find_package(joint_state_broadcaster REQUIRED)
find_package(joint_trajectory_controller REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(moveit_ros_move_group REQUIRED)
find_package(gz_ros2_control REQUIRED)
find_package(ros_gz_sim REQUIRED)


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

add_executable(arm_controller
  src/arm_controller.cpp
)
add_executable(move_group_controller
  src/move_group_controller.cpp
)

add_executable(robot_motion_controller
  src/robot_motion_controller.cpp
) 

ament_target_dependencies(arm_controller
  rclcpp
  rclcpp_action
  control_msgs
  trajectory_msgs
)

ament_target_dependencies(move_group_controller
  rclcpp
  moveit_ros_planning_interface
)

ament_target_dependencies(robot_motion_controller
  rclcpp
  moveit_ros_move_group
)

install(
  TARGETS arm_controller move_group_controller robot_motion_controller
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY launch config urdf worlds
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
