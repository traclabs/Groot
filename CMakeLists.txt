cmake_minimum_required(VERSION 3.16)

project(groot)

find_package(Qt5 COMPONENTS  Core Widgets Gui OpenGL Xml Svg)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}  "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

find_package(ament_cmake QUIET)
find_package(ament_index_cpp REQUIRED)

set(dependencies
  rclcpp
  behaviortree_cpp
)

foreach(dependency IN ITEMS ${dependencies})
  find_package(${dependency} REQUIRED)
endforeach()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_definitions(${Qt5Widgets_DEFINITIONS} -DNODE_EDITOR_STATIC )
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

include_directories(include ./QtNodeEditor/include ./)

add_subdirectory( QtNodeEditor )


set(APP_CPPS
    ./bt_editor/models/BehaviorTreeNodeModel.cpp
    ./bt_editor/models/SubtreeNodeModel.cpp

    ./bt_editor/mainwindow.cpp
    ./bt_editor/editor_flowscene.cpp
    ./bt_editor/utils.cpp
    ./bt_editor/bt_editor_base.cpp
    ./bt_editor/graphic_container.cpp
    ./bt_editor/startup_dialog.cpp

    ./bt_editor/sidepanel_editor.cpp
    ./bt_editor/sidepanel_replay.cpp
    ./bt_editor/custom_node_dialog.cpp

    ./bt_editor/XML_utilities.cpp
    )

set(FORMS_UI
  ./bt_editor/about_dialog.ui
  ./bt_editor/mainwindow.ui
  ./bt_editor/action_form.ui
  ./bt_editor/sidepanel_editor.ui
  ./bt_editor/sidepanel_replay.ui
  ./bt_editor/startup_dialog.ui
  ./bt_editor/custom_node_dialog.ui
  )

find_package(ZMQ)

if( ZMQ_FOUND )
    message(STATUS "ZeroMQ found.")
    add_definitions( -DZMQ_FOUND )

    set(APP_CPPS ${APP_CPPS} ./bt_editor/sidepanel_monitor.cpp )
    set(FORMS_UI ${FORMS_UI} ./bt_editor/sidepanel_monitor.ui )

else()
    message(WARNING "ZeroMQ NOT found. Real-time monitoring disabled.")
endif()


set(RESOURCE_FILES
  bt_editor/resources/icons.qrc
  bt_editor/resources/style.qrc
  test_data/test_files.qrc
)

QT5_WRAP_UI(FORMS_HEADERS ${FORMS_UI})

add_library(behavior_tree_editor SHARED
    ${APP_CPPS}
    ${FORMS_HEADERS}
    ${RESOURCE_FILES}
)

SET(GROOT_DEPENDENCIES QtNodeEditor ncurses ncursesw tinfo Qt5::Core Qt5::Svg )

ament_target_dependencies(behavior_tree_editor ${dependencies})

if( ZMQ_FOUND )
    if (APPLE)
        find_package(cppzmq)
        SET(GROOT_DEPENDENCIES ${GROOT_DEPENDENCIES} cppzmq)
    else()
        SET(GROOT_DEPENDENCIES ${GROOT_DEPENDENCIES} zmq)
    endif()
endif()

target_link_libraries(behavior_tree_editor ${GROOT_DEPENDENCIES} )


#add_executable(Groot ./bt_editor/main.cpp ${RESOURCE_FILES})
#target_link_libraries(Groot behavior_tree_editor )

add_executable(groot_editor_node nodes/groot_editor_node.cpp)
target_link_libraries(groot_editor_node behavior_tree_editor)
ament_target_dependencies(groot_editor_node ${dependencies})

add_executable(groot_monitor_node nodes/groot_monitor_node.cpp)
target_link_libraries(groot_monitor_node behavior_tree_editor)
ament_target_dependencies(groot_monitor_node ${dependencies})


add_subdirectory(test)

######################################################
# INSTALL
set( GROOT_LIB_DESTINATION   lib )
set( GROOT_INC_DESTINATION   include )
set( GROOT_BIN_DESTINATION   lib/${PROJECT_NAME} )

INSTALL(TARGETS 
 behavior_tree_editor
 groot_editor_node
 groot_monitor_node
# Groot 
 LIBRARY DESTINATION ${GROOT_LIB_DESTINATION} 
 RUNTIME DESTINATION ${GROOT_BIN_DESTINATION} 
)

install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_dependencies(${GROOT_DEPENDENCIES} ${dependencies})
ament_export_libraries(behavior_tree_editor)
ament_package()

