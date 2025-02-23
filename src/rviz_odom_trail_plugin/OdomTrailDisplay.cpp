#include "rviz_odom_trail_plugin/OdomTrailDisplay.h"

namespace rviz_odom_trail_plugin
{

OdomTrailDisplay::OdomTrailDisplay()
  : trail_(nullptr)
  , sphere_(nullptr)
  , have_last_point_(false)
  , text_node_(nullptr)
  , text_object_(nullptr)
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

  // 3) Initialize properties for 3D text
  show_text_property_ = new rviz::BoolProperty(
      "Show Text",
      true,  // default: show text
      "Enable/disable a text label at the latest position",
      this,
      SLOT(updateText()));

  text_string_property_ = new rviz::StringProperty(
      "Text",
      "", // "DEFAULTTEXT",
      "Text to display (leave empty if you do not want any label)",
      this,
      SLOT(updateText()));

  text_color_property_ = new rviz::ColorProperty(
      "Text Color",
      QColor(20, 20, 20), // default = dark gray
      "Color of the text label",
      this,
      SLOT(updateText()));

  text_alpha_property_ = new rviz::FloatProperty(
      "Text Alpha",
      1.0,
      "Alpha (transparency) of the text label",
      this,
      SLOT(updateText()));
  text_alpha_property_->setMin(0.0);
  text_alpha_property_->setMax(1.0);

  text_scale_property_ = new rviz::FloatProperty(
      "Text Scale",
      0.1,
      "Height of the characters (in meters)",
      this,
      SLOT(updateText()));
  text_scale_property_->setMin(0.0);

  text_offset_property_ = new rviz::VectorProperty(
      "Text Offset",
      Ogre::Vector3(0.0f, 0.0f, 0.1f),
      "XYZ offset of the text label from the last point",
      this,
      SLOT(updateText()));
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

  if (text_object_)
  {
    // text_object_ is attached to text_node_, so just delete the object
    delete text_object_;
    text_object_ = nullptr;
  }

  if (text_node_)
  {
    // The scene node was created by us, so remove it from the scene
    scene_manager_->destroySceneNode(text_node_);
    text_node_ = nullptr;
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

  // Create a dedicated SceneNode for the text
  text_node_ = scene_node_->createChildSceneNode();

  // Create a MovableText object with an empty string initially
  text_object_ = new rviz::MovableText("default"); // Empty string doesn't work!!
  
  text_object_->setVisible(false); // Initially hidden // DOESN'T hide it

  Ogre::ColourValue color = rviz::qtToOgre(text_color_property_->getColor());
  color.a = 0.0; // Initially invisible // Finally, this works to hide it initially
  text_object_->setColor(color);

  // text_object_->setFontName("Liberation Sans");
  // text_object_->setFontName("Arial");
  
  text_object_->setTextAlignment(rviz::MovableText::H_CENTER,
    rviz::MovableText::V_ABOVE);
    
  // text_object_->showOnTop(false); // If true, it will always render on top
  text_object_->showOnTop(true);
  text_node_->attachObject(text_object_);


  // Make sure the sphere is visible or hidden initially
  updateSphere();

  // Set initial line properties
  updateLine();

  // Apply any current property settings
  updateText();
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

  // Also move the text
  if (text_node_ && have_last_point_)
  {
    Ogre::Vector3 offset = text_offset_property_->getVector();
    text_node_->setPosition(last_point_ + offset);
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

void OdomTrailDisplay::updateText()
{
  // Called when user toggles showing text or changes text properties
  if (!text_node_ || !text_object_){
      return;
  }

  // 1) Toggle visibility
  bool show_text = show_text_property_->getBool();
  std::string user_txt = text_string_property_->getStdString();

  // If user sets Show Text = false OR text is empty, just hide it.
  if (!show_text || user_txt.empty() || (user_txt.find_first_not_of(" \t\n\r\f\v") == std::string::npos))
  {
    text_object_->setVisible(false);
    return;
  }

  // Otherwise, show text and update
  text_object_->setVisible(true);
  text_object_->setCaption(user_txt);

  // 3) Set color + alpha
  Ogre::ColourValue color = rviz::qtToOgre(text_color_property_->getColor());
  float alpha = text_alpha_property_->getFloat();
  color.a = alpha;
  text_object_->setColor(color);

  // 4) Scale
  float char_height = text_scale_property_->getFloat();
  text_object_->setCharacterHeight(char_height);

  // 5) Offset: If we already have a last_point_, place the text there
  if (have_last_point_)
  {
    Ogre::Vector3 offset = text_offset_property_->getVector();
    text_node_->setPosition(last_point_ + offset);
  }
}

} // end namespace rviz_odom_trail_plugin

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(rviz_odom_trail_plugin::OdomTrailDisplay, rviz::Display)

