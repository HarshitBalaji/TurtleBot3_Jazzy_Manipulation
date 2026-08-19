#!/usr/bin/env python3

import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command
from launch_ros.actions import Node


def generate_launch_description():

    sim_share = get_package_share_directory(
        'turtlebot3_manipulation_sim'
    )

    robot_xacro = os.path.join(
        sim_share,
        'urdf',
        'turtlebot3_manipulation_gz.urdf.xacro'
    )

    world = os.path.join(
        sim_share,
        'worlds',
        'empty.sdf'
    )

    robot_description = Command([
        'xacro',
        ' ',
        robot_xacro
    ])

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('ros_gz_sim'),
                'launch',
                'gz_sim.launch.py'
            )
        ),
        launch_arguments={
            'gz_args': ['-r ', world]
        }.items()
    )

    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[
            {'robot_description': robot_description}
        ],
        output='screen'
    )

    spawn_robot = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=[
            '-topic', 'robot_description',
            '-name', 'turtlebot3_manipulation',
            '-x', '0.0',
            '-y', '0.0',
            '-z', '0.05',
        ],
        output='screen'
    )

    return LaunchDescription([
        gazebo,
        robot_state_publisher,
        spawn_robot,
    ])
