#pragma once

// #ifndef RVIZ_ODOM_TRAIL_PLUGIN_ODOM_TRAIL_DISPLAY_H
// #define RVIZ_ODOM_TRAIL_PLUGIN_ODOM_TRAIL_DISPLAY_H

#ifndef Q_MOC_RUN
// #include <OGRE/OgreSceneNode.h>
// #include <OGRE/OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

// #include <tf/transform_listener.h>

#include <rviz/properties/color_property.h>
#include <rviz/properties/float_property.h>
#include <rviz/properties/int_property.h>
#include <rviz/properties/bool_property.h>

// #include <rviz/visualization_manager.h>
// #include <rviz/frame_manager.h>

#include <rviz/ogre_helpers/billboard_line.h>
#include <rviz/ogre_helpers/shape.h>

#include <rviz/message_filter_display.h>
#include <nav_msgs/Odometry.h>

#endif

#include <vector>


// Forward declarations to reduce compile time
namespace rviz
{
class ColorProperty;
class FloatProperty;
class IntProperty;
class BoolProperty;

// Helpers for drawing lines & shapes in RViz
class BillboardLine;
class Shape;
}  // namespace rviz

namespace rviz_odom_trail_plugin
{

/**
 * @class OdomTrailDisplay
 * @brief Displays a trail (path) and an optional sphere for the last position
 *        from a stream of nav_msgs::Odometry messages.
 */
class OdomTrailDisplay : public rviz::MessageFilterDisplay<nav_msgs::Odometry>
{
Q_OBJECT
public:
  OdomTrailDisplay();
  ~OdomTrailDisplay() override;

protected:
  /**
   * @brief Called once the display is initialized. We create all the
   *        RViz-specific render objects and properties here.
   */
  void onInitialize() override;

  /**
   * @brief Called when the user or something else resets the display. 
   *        We clean up data structures here.
   */
  void reset() override;

  /**
   * @brief Main callback for incoming messages. This is where we 
   *        parse the odometry and update the trail.
   */
  void processMessage(const nav_msgs::OdometryConstPtr &msg) override;

private Q_SLOTS:
  /**
   * @brief Update callbacks for the RViz Property system
   */
  void updateLine();
  void updateSphere();

private:
  // Trail (line) properties
  rviz::ColorProperty* line_color_property_;
  rviz::FloatProperty* line_alpha_property_;
  rviz::FloatProperty* line_width_property_;
  rviz::IntProperty*   max_points_property_;
  rviz::FloatProperty* min_distance_property_;

  // Sphere (latest position) properties
  rviz::BoolProperty*  show_sphere_property_;
  rviz::ColorProperty* sphere_color_property_;
  rviz::FloatProperty* sphere_alpha_property_;
  rviz::FloatProperty* sphere_scale_property_;

  // Internal data structures
  std::vector<Ogre::Vector3> path_points_;
  rviz::BillboardLine* trail_;   ///< Helper class for rendering a polyline
  rviz::Shape*         sphere_;  ///< Helper class for rendering a sphere

  Ogre::Vector3 last_point_;
  bool          have_last_point_;
};

}  // end namespace rviz_odom_trail_plugin

// #endif  // RVIZ_ODOM_TRAIL_PLUGIN_ODOM_TRAIL_DISPLAY_H
