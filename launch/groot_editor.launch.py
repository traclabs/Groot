import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution, LaunchConfiguration, PythonExpression
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from ament_index_python.packages import get_package_share_directory
import xacro


def generate_launch_description():  
  
  launch_args = [
    DeclareLaunchArgument("use_sim_time", default_value="False"),
    DeclareLaunchArgument("model_files", default_value="[]"),
    DeclareLaunchArgument("bt_file", default_value="")    
  ]
  
  groot_editor = Node(
        package="groot",
        executable="groot_editor_node",
        name="groot",
        parameters=[
          {"use_sim_time": LaunchConfiguration("use_sim_time")},
          {"model_files": PythonExpression(LaunchConfiguration("model_files"))},
          {"bt_file": LaunchConfiguration("bt_file")}
        ],
        output="screen",
        #prefix=['xterm -e gdb -ex run --args']
  )
   
  
  return LaunchDescription(
    launch_args + [
      groot_editor
    ]
  )
