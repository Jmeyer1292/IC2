#ifndef CAL_DATA_COLLECTOR_H
#define CAL_DATA_COLLECTOR_H

#include <boost/thread.hpp>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv_modules.hpp>
#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <yaml-cpp/yaml.h>

static boost::mutex MUTEX;

class CalDataCollector
{
public:
  CalDataCollector(ros::NodeHandle nh, ros::NodeHandle pnh);

  ~CalDataCollector(void) { }

  void collectData(void);

// Private Methods
private:
  bool drawGrid(const cv::Mat &input_image, cv::Mat &output_image);

  void imageCallback(const sensor_msgs::ImageConstPtr &msg);

  void mouseCallbackInternal(int event, int x, int y, int flags);

  static void mouseCallback(int event, int x, int y, int flags, void* param);

  void initDisplayWindow(const std::string &window_name);

  bool checkSettings(void);

// Private Variables
private:
  ros::NodeHandle nh_;
  ros::NodeHandle pnh_;
  image_transport::ImageTransport image_transport;
  image_transport::Subscriber image_subscriber;
  std::string cv_window_name_;
  std::string save_path_;
  cv::Mat raw_image_;
  cv::Mat grid_image_;
  int pattern_cols_;
  int pattern_rows_;
  bool exit_;

};
#endif // CAL_DATA_COLLECTOR_H