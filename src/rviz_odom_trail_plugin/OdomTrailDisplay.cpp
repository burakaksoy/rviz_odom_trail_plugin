#include "rviz_odom_trail_plugin/OdomTrailDisplay.h"

namespace rviz_odom_trail_plugin
{

OdomTrailDisplay::OdomTrailDisplay()
  : trail_(nullptr)
  , sphere_(nullptr)
  , have_last_point_(false)
{
  // --------------------------------------------------------------------------
  // 1) Initialize properties for the line (the path trail)
  // --------------------------------------------------------------------------
  line_color_property_ = new rviz::ColorProperty(
      "Line Color",
      QColor(255, 0, 0), // default = red
      "Color of the trail",
      this,
      SLOT(updateLine()));

  line_alpha_property_ = new rviz::FloatProperty(
      "Line Alpha",
      1.0, // default = fully opaque
      "Alpha (transparency) of the trail",
      this,
      SLOT(updateLine()));
  line_alpha_property_->setMin(0.0);
  line_alpha_property_->setMax(1.0);

  line_width_property_ = new rviz::FloatProperty(
      "Line Width",
      0.05,
      "Width of the trail (in meters)",
      this,
      SLOT(updateLine()));
  line_width_property_->setMin(0.001); // some small positive number

  max_points_property_ = new rviz::IntProperty(
      "Max Points",
      1000,
      "Maximum number of points in the path to display",
      this);
  max_points_property_->setMin(1);

  min_distance_property_ = new rviz::FloatProperty(
      "Min Distance",
      0.001,
      "Minimum distance between consecutive points before adding a new one",
      this,
      SLOT(updateLine()));
  min_distance_property_->setMin(0.0);

  // --------------------------------------------------------------------------
  // 2) Initialize properties for the sphere (showing last point)
  // --------------------------------------------------------------------------
  show_sphere_property_ = new rviz::BoolProperty(
      "Show Sphere",
      true,
      "Enable/disable showing a sphere at the latest position",
      this,
      SLOT(updateSphere()));

  sphere_color_property_ = new rviz::ColorProperty(
      "Sphere Color",
      QColor(0, 255, 0), // default = green
      "Color of the sphere at the latest position",
      this,
      SLOT(updateSphere()));

  sphere_alpha_property_ = new rviz::FloatProperty(
      "Sphere Alpha",
      1.0,
      "Alpha (transparency) of the sphere",
      this,
      SLOT(updateSphere()));
  sphere_alpha_property_->setMin(0.0);
  sphere_alpha_property_->setMax(1.0);

  sphere_scale_property_ = new rviz::FloatProperty(
      "Sphere Scale",
      0.1,
      "Diameter of the sphere (in meters) for the latest position",
      this,
      SLOT(updateSphere()));
  sphere_scale_property_->setMin(0.0);
}

OdomTrailDisplay::~OdomTrailDisplay()
{
  // Clean up any allocated resources
  if (trail_)
  {
    delete trail_;
    trail_ = nullptr;
  }
  if (sphere_)
  {
    delete sphere_;
    sphere_ = nullptr;
  }
}

void OdomTrailDisplay::onInitialize()
{
  // Called once RViz has set up the scene node, etc.
  MFDClass::onInitialize();

  // Create our trail (a BillboardLine) for displaying the path
  trail_ = new rviz::BillboardLine(context_->getSceneManager(), scene_node_);

  // Create our sphere for displaying the last point
  sphere_ = new rviz::Shape(
      rviz::Shape::Sphere, 
      context_->getSceneManager(), 
      scene_node_);

  // Make sure the sphere is visible or hidden initially
  updateSphere();

  // Set initial line properties
  updateLine();
}

void OdomTrailDisplay::reset()
{
  // Called if the display is "reset" via the context or user
  MFDClass::reset();

  path_points_.clear();
  if (trail_)
  {
    trail_->clear();
  }
  have_last_point_ = false;
}

void OdomTrailDisplay::processMessage(const nav_msgs::OdometryConstPtr& msg)
{
  if (!msg)
  {
    return;
  }

  // Extract the new position from the odometry
  const auto& pos = msg->pose.pose.position;
  Ogre::Vector3 new_point(pos.x, pos.y, pos.z);

  // If we don't have a 'last_point_' yet, initialize it
  if (!have_last_point_)
  {
    last_point_     = new_point;
    have_last_point_ = true;
    path_points_.push_back(new_point);
  }
  else
  {
    float dist = (new_point - last_point_).length();
    float min_dist = min_distance_property_->getFloat();
    // Only add a new point if it moved more than the min distance
    if (dist >= min_dist)
    {
      path_points_.push_back(new_point);
      last_point_ = new_point;
    }
  }

  // Enforce maximum number of points
  int max_pts = max_points_property_->getInt();
  while (static_cast<int>(path_points_.size()) > max_pts)
  {
    path_points_.erase(path_points_.begin());
  }

  // --------------------------------------------------------------------------
  // Update the trail
  // --------------------------------------------------------------------------
  if (trail_)
  {
    // Clear and re-draw
    trail_->clear();
    trail_->setNumLines(1);
    trail_->setMaxPointsPerLine(path_points_.size());
    trail_->setLineWidth(line_width_property_->getFloat());

    // Convert the line color property to Ogre color
    Ogre::ColourValue line_color = rviz::qtToOgre(line_color_property_->getColor());
    line_color.a = line_alpha_property_->getFloat();

    // Add each point in the path
    for (size_t i = 0; i < path_points_.size(); ++i)
    {
      trail_->addPoint(path_points_[i], line_color);
    }
  }

  // --------------------------------------------------------------------------
  // Update the sphere (latest position)
  // --------------------------------------------------------------------------
  if (sphere_ && have_last_point_)
  {
    sphere_->setPosition(last_point_);
  }
}

void OdomTrailDisplay::updateLine()
{
  // Whenever a property relevant to the line changes, you can
  // re-apply it here (e.g. color, alpha, width, etc.)
  if (!trail_)
  {
    return;
  }

  Ogre::ColourValue color = rviz::qtToOgre(line_color_property_->getColor());
  color.a = line_alpha_property_->getFloat();
  trail_->setLineWidth(line_width_property_->getFloat());

  // If you'd like to immediately re-color the existing points, you can do so
  // but in many cases it's enough to wait until processMessage() re-draws.
}

void OdomTrailDisplay::updateSphere()
{
  // Called when user toggles showing sphere or changes sphere properties
  if (!sphere_)
  {
    return;
  }

  bool show_sphere = show_sphere_property_->getBool();
  sphere_->getRootNode()->setVisible(show_sphere);

  // Update sphere color
  Ogre::ColourValue color = rviz::qtToOgre(sphere_color_property_->getColor());
  color.a = sphere_alpha_property_->getFloat();
  sphere_->setColor(color.r, color.g, color.b, color.a);

  // Scale the sphere
  float diameter = sphere_scale_property_->getFloat();
  // shape->setScale is a scaling factor on the unit sphere,
  // so we use (diameter, diameter, diameter).
  sphere_->setScale(Ogre::Vector3(diameter, diameter, diameter));
}

} // end namespace rviz_odom_trail_plugin

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(rviz_odom_trail_plugin::OdomTrailDisplay, rviz::Display)

