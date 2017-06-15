#ifndef OBSERVATIONS_H
#define OBSERVATIONS_H

#include <industrial_calibration_libs/targets.h>
#include <industrial_calibration_libs/circle_detector.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace industrial_calibration_libs
{
#if 0 
struct Observation
{
  std::shared_ptr<Target> target;
  int point_id;
  double point_x;
  double point_y;
  // Note(gChiou): May need cost function type associated with observation and
  // an 'intermediate_frame'. Leaving it out for now.
  // see: https://github.com/ros-industrial/industrial_calibration/blob/kinetic-devel/industrial_extrinsic_cal/include/industrial_extrinsic_cal/camera_observer.hpp
};
#endif

// Note(gChiou): Was Point2f
typedef std::vector<cv::Point3f> ObservationPoints;

// Note(gChiou): I think the above typedef is for a single image.
// The input of the ObservationExtractor class should be a vector of 
// all images containing a target and the output should be a vector of:
// typedef std::vector<CameraObservations> ObservationData 
// (or something like that) of the same size as input vector of images.
// Something to test for.
typedef std::vector<ObservationPoints> ObservationData;

class ObservationExtractor
{
public:
  ObservationExtractor(const std::vector<cv::Mat> &images, const Target &target);

  ~ObservationExtractor(void);

  ObservationData getObservationData(void) {return observation_data_;}

  // TODO(gChiou): Put this in constructor.
  bool extractObservations(void);

private:
  bool checkData(void) const;

  bool extractChessboard(void);

  bool extractCircleGridAsymmetric(void);

  bool extractCircleGridSymmetric(void);

  bool extractModifiedCircleGrid(void);

  // Data Members
  std::vector<cv::Mat> images_;
  ObservationData observation_data_;
  Target target_;
  bool custom_circle_detector_;
};

} // namespace industrial_calibration_libs
#endif // OBSERVATIONS_H

