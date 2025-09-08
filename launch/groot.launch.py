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
  
  bt_file = os.path.join(get_package_share_directory("bt_tools_craftsman"), "behaviors", "test_fetch_ic.xml")
  groot = Node(
        package="groot",
        executable="groot_editor_node",
        name="groot",
        parameters=[
          {"use_sim_time": LaunchConfiguration("use_sim_time")},
#          {"bt_file": bt_file}
        ],
        output="screen",
        #prefix=['xterm -e gdb -ex run --args']
  )
   
  
  return LaunchDescription(
    launch_args + [
      groot
    ]
  )
