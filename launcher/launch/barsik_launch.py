from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='turret_controller',
            executable='turret_controller',
            name='turret'
        ),
        Node(
            package='fisheye-controller',
            # namespace='fisheye-camera',
            executable='fisheye-controller',
            name='fish'
        ),
        Node(
            package='eagle-eye-controller',
            executable='eagle-eye-controller',
            name='eagle'
        ),
        Node(
            package='coordinator',
            executable='coordinator',
            name='coordinator'
        )
    ])
