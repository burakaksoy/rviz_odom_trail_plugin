/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2025, Rensselaer Polytechnic Institute, Burak Aksoy (burakaksoy20@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Author: Burak Aksoy
 */

#pragma once

// #ifndef RVIZ_ODOM_TRAIL_PLUGIN_ODOM_TRAIL_DISPLAY_H
// #define RVIZ_ODOM_TRAIL_PLUGIN_ODOM_TRAIL_DISPLAY_H

#ifndef Q_MOC_RUN
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreManualObject.h>
// #include <OgreSceneNode.h>
// #include <OgreSceneManager.h>

// #include <tf/transform_listener.h>

#include <rviz/properties/color_property.h>
#include <rviz/properties/float_property.h>
#include <rviz/properties/int_property.h>
#include <rviz/properties/bool_property.h>
#include <rviz/properties/vector_property.h> 

// #include <rviz/visualization_manager.h>
// #include <rviz/frame_manager.h>

#include <rviz/ogre_helpers/billboard_line.h>
#include <rviz/ogre_helpers/movable_text.h>
#include <rviz/ogre_helpers/shape.h>

#include <rviz/message_filter_display.h>
#include <nav_msgs/Odometry.h>

#include <vector>
#include <string>
#include <iostream>
#include <ros/ros.h>

#endif

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
  void updateText();

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

  // --------------------------------------------------------
  // 3D Text Properties
  // --------------------------------------------------------
  rviz::BoolProperty*   show_text_property_;   ///< Enable/disable text display
  rviz::StringProperty* text_string_property_; ///< Actual text string
  rviz::ColorProperty*  text_color_property_;  ///< (R,G,B)
  rviz::FloatProperty*  text_alpha_property_;  ///< alpha
  rviz::FloatProperty*  text_scale_property_;  ///< character height
  rviz::VectorProperty* text_offset_property_; ///< offset from last point

  // Internal data structures
  std::vector<Ogre::Vector3> path_points_;
  rviz::BillboardLine* trail_;   ///< Helper class for rendering a polyline
  rviz::Shape*         sphere_;  ///< Helper class for rendering a sphere

  Ogre::Vector3 last_point_;
  bool          have_last_point_;

  // --------------------------------------------------------
  // 3D Text Rendering
  // --------------------------------------------------------
  Ogre::SceneNode*      text_node_;
  rviz::MovableText*    text_object_;
};

}  // end namespace rviz_odom_trail_plugin

// #endif  // RVIZ_ODOM_TRAIL_PLUGIN_ODOM_TRAIL_DISPLAY_H
