cmake_minimum_required(VERSION 3.0.2)
project(rviz_odom_trail_plugin)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
rviz
roscpp
nav_msgs
# message_filters
# roslib
# pluginlib
)

catkin_package(
  INCLUDE_DIRS include
  CATKIN_DEPENDS roscpp rviz pluginlib nav_msgs
  LIBRARIES rviz_odom_trail_plugin
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  
  # ${CMAKE_CURRENT_SOURCE_DIR}/include
  # ${Qt5Widgets_INCLUDE_DIRS}
)

#
# Build rviz plugins
#
# Enable AUTOMOC so that the moc file for any Q_OBJECT class is generated.
set(CMAKE_AUTOMOC ON)

## This plugin includes Qt widgets, so we must include Qt.
## We'll use the version that rviz used so they are compatible.
if(rviz_QT_VERSION VERSION_LESS "5")
  message(STATUS "Using Qt4 based on the rviz_QT_VERSION: ${rviz_QT_VERSION}")
  find_package(Qt4 ${rviz_QT_VERSION} EXACT REQUIRED QtCore QtGui)
  ## pull in all required include dirs, define QT_LIBRARIES, etc.
  include(${QT_USE_FILE})
else()
  message(STATUS "Using Qt5 based on the rviz_QT_VERSION: ${rviz_QT_VERSION}")
  find_package(Qt5 ${rviz_QT_VERSION} EXACT REQUIRED Core Widgets)
  ## make target_link_libraries(${QT_LIBRARIES}) pull in all required dependencies
  set(QT_LIBRARIES Qt5::Widgets)
endif()

add_definitions(-DQT_NO_KEYWORDS)


add_library(rviz_odom_trail_plugin
  src/rviz_odom_trail_plugin/OdomTrailDisplay.cpp 
  include/rviz_odom_trail_plugin/OdomTrailDisplay.h
)

target_link_libraries(rviz_odom_trail_plugin
  ${catkin_LIBRARIES}
  ${QT_LIBRARIES}
)

add_dependencies(rviz_odom_trail_plugin ${catkin_EXPORTED_TARGETS})


#
# Install libraries
#

# # Install the shared library into the lib folder.
install(TARGETS rviz_odom_trail_plugin
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})

#
# Install header files
#

# # Install the public headers.
# install(DIRECTORY include/${PROJECT_NAME}/
#   DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})

install(DIRECTORY include/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})


#
# Install nodelet and rviz plugins description
#
# # Install the plugin description XML.
install(FILES plugin_description.xml
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

# #
# # Install launch files
# #
# install(DIRECTORY launch/
#   DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

# #
# # Create folders and copy resources
# #
# file(MAKE_DIRECTORY $ENV{HOME}/.local/share/icons/robor)
# file(COPY resources/play.png resources/stop.png DESTINATION $ENV{HOME}/.local/share/icons/robor)





