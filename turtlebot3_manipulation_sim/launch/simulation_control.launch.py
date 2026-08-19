#!/usr/bin/env python3

import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    ExecuteProcess,
    IncludeLaunchDescription,
    RegisterEventHandler,
    SetEnvironmentVariable,
    TimerAction,
)
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():

    sim_pkg = get_package_share_directory(
        'turtlebot3_manipulation_sim'
    )

    moveit_pkg = get_package_share_directory(
        'turtlebot3_manipulation_moveit_config'
    )

    description_pkg = get_package_share_directory(
        'turtlebot3_manipulation_description'
    )

    controllers_yaml = os.path.join(
        sim_pkg,
        'config',
        'controllers.yaml'
    )

    gazebo_launch = os.path.join(
        sim_pkg,
        'launch',
        'gazebo.launch.py'
    )

    move_group_launch = os.path.join(
        moveit_pkg,
        'launch',
        'move_group.launch.py'
    )

    rviz_launch = os.path.join(
        moveit_pkg,
        'launch',
        'moveit_rviz.launch.py'
    )

    # Gazebo needs the installed robot-description resources.
    gz_resource_path = os.path.join(
        description_pkg,
        ''
    )

    set_gz_resource_path = SetEnvironmentVariable(
        name='GZ_SIM_RESOURCE_PATH',
        value=[
            gz_resource_path,
            ':',
            os.environ.get('GZ_SIM_RESOURCE_PATH', '')
        ]
    )

    use_sim_time = LaunchConfiguration('use_sim_time')

    declare_use_sim_time = DeclareLaunchArgument(
        'use_sim_time',
        default_value='true',
        description='Use Gazebo simulation time.'
    )

    # 1. Start Gazebo.
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(gazebo_launch)
    )

    # 2. Spawn the controllers sequentially.
    #
    # Each spawner waits for /controller_manager to become available.
    # The shell command exits only after all three controllers are active.
    controller_command = (
        f"ros2 run controller_manager spawner joint_state_broadcaster "
        f"--controller-manager /controller_manager "
        f"--param-file {controllers_yaml} && "
        f"ros2 run controller_manager spawner arm_controller "
        f"--controller-manager /controller_manager "
        f"--param-file {controllers_yaml} && "
        f"ros2 run controller_manager spawner gripper_controller "
        f"--controller-manager /controller_manager "
        f"--param-file {controllers_yaml}"
    )

    spawn_controllers = ExecuteProcess(
        cmd=['bash', '-c', controller_command],
        output='screen'
    )

    # 3. Start MoveGroup after all controllers are ready.
    move_group = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(move_group_launch),
        launch_arguments={
            'use_sim': use_sim_time
        }.items()
    )

    # 4. Start RViz after MoveGroup has had time to initialize.
    rviz = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(rviz_launch)
    )

    # 5. Start our C++ application node after the complete MoveIt stack
    # has had time to initialize.
    controller_node = Node(
        package='turtlebot3_manipulation_sim',
        executable='move_group_controller',
        name='turtlebot3_manipulation_controller',
        output='screen',
        parameters=[
            {'use_sim_time': use_sim_time}
        ]
    )

    start_move_group_after_controllers = RegisterEventHandler(
        OnProcessExit(
        target_action=spawn_controllers,
        on_exit=[
            move_group,
            TimerAction(
                period=5.0,
                actions=[rviz]
            ),
            TimerAction(
                period=10.0,
                actions=[controller_node]
            )
        ]
        )
    )

    return LaunchDescription([
        declare_use_sim_time,
        set_gz_resource_path,
        gazebo,
        spawn_controllers,
        start_move_group_after_controllers
    ])

