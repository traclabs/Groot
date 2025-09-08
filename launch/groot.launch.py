import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from ament_index_python.packages import get_package_share_directory
import xacro


def generate_launch_description():  
  
  launch_args = [
    DeclareLaunchArgument("use_sim_time", default_value="False")
  ]
  
  groot = Node(
        package="groot",
        executable="groot_editor_node",
        name="groot",
        parameters=[
          {"use_sim_time": LaunchConfiguration("use_sim_time")}
        ],
        output="screen",
        #prefix=['xterm -e gdb -ex run --args']
  )
   
  
  return LaunchDescription(
    launch_args + [
      groot
    ]
  )
