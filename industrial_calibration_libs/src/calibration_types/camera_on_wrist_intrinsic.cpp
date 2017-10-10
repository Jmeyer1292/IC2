#include <industrial_calibration_libs/calibration_types/camera_on_wrist_intrinsic.h>

namespace industrial_calibration_libs
{
CameraOnWristIntrinsic::CameraOnWristIntrinsic(const ObservationData &observation_data, const Target &target, const CameraOnWristIntrinsicParams &params,
  const CameraOnWristIntrinsicMethod &method) 
  : CalibrationJob(observation_data, target), method_(method)
{
  link_poses_ = params.base_to_tool;
  std::memcpy(result_.intrinsics, params.intrinsics.data, 
    sizeof(result_.intrinsics));
  std::memcpy(result_.target_to_camera, params.target_to_camera.data,
    sizeof(result_.target_to_camera));
}

bool CameraOnWristIntrinsic::runCalibration(void)
{
  if (link_poses_.size() == 0)
  {
    std::cerr << "base to tool Poses are Empty!" << '\n';
    return false;
  }
  
  if (!checkObservations()) {return false;}

  switch (method_)
  {
    case CALCULATE_FIRST_POSE:
      return this->runCalculateFirstPose();
      break;

    case CALCULATE_EVERY_POSE:
      return this->runCalculateEveryPose();
      break;

    default:
      return false;
  }

  return false;
}

bool CameraOnWristIntrinsic::calculateFirstPose(void)
{
  Pose6D target_to_position1; // Position of first calibration image
  Pose6D target_to_position2; // Position of last calibration image
  Pose6D position2_to_target;
  Pose6D position2_to_position1;

  // Calculate target_to_position1 and target_to_position2
  this->findDistortedTarget(observation_data_[0], target_to_position1);
  this->findDistortedTarget(observation_data_[num_images_-1], target_to_position2);

  position2_to_target = target_to_position2.getInverse();
  position2_to_position1 = target_to_position1 * position2_to_target;
  // Get vector along which target moves, might not need this.

  // Iterate through every observation image
  for (std::size_t i = 0; i < num_images_; i++)
  {
    // Iterate through every observation in each observation image
    for (std::size_t j = 0; j < observations_per_image_; j++)
    {
      Pose6D link_pose = link_poses_[i];
      Point3D point = target_.getDefinition().points[j];

      double observed_x = observation_data_[i][j].x;
      double observed_y = observation_data_[i][j].y;

      ceres::CostFunction *cost_function =
        CameraOnWristIntrinsicCF::Create(observed_x, observed_y,
          link_pose, point);

      problem_.AddResidualBlock(cost_function, NULL, result_.intrinsics,
        result_.target_to_camera);
    }
  }

  // Solve
  options_.linear_solver_type = ceres::DENSE_SCHUR;
  options_.minimizer_progress_to_stdout = output_results_; 
  options_.max_num_iterations = 9001;

  ceres::Solve(options_, &problem_, &summary_);

  if (output_results_)
  {
    std::cout << summary_.FullReport() << '\n';  
  }

  if (summary_.termination_type != ceres::NO_CONVERGENCE)
  {
    initial_cost_ = summary_.initial_cost / total_observations_;
    final_cost_ = summary_.final_cost / total_observations_;
    return true;    
  }

  return false;
}

bool CameraOnWristIntrinsic::runCalculateEveryPose(void)
{
  return false;
}

bool CameraOnWristIntrinsic::findDistortedTarget(const ObservationPoints &observation_points,
  Pose6D &result)
{ 
  double focal_x = result_.intrinsics[0];
  double focal_y = result_.intrinsics[1];
  double optical_center_x = result_.intrinsics[2];
  double optical_center_y = result_.intrinsics[3];
  double distortion_k1 = result_.intrinsics[4];
  double distortion_k2 = result_.intrinsics[5];
  double distortion_k3 = result_.intrinsics[6];
  double distortion_p1 = result_.intrinsics[7];
  double distortion_p2 = result_.intrinsics[8];

  // Set initial conditions
  Pose6D guess_pose(result_.target_to_camera[3], result_.target_to_camera[4],
    result_.target_to_camera[5], result_.target_to_camera[0], 
    result_.target_to_camera[1], result_.target_to_camera[2]);

  double result_pose[6];
  // Copy the initial guess into solver input.
  std::memcpy(result_pose, result_.target_to_camera, 
    sizeof(result_pose));

  ceres::Problem problem;
  for (std::size_t i = 0; i < observations_per_image_; i++)
  {
    Point3D point = target_.getDefinition().points[i];

    double observed_x = observation_points[i].x;
    double observed_y = observation_points[i].y;

    ceres::CostFunction *cost_function = 
      DistortedTargetFinder::Create(observed_x, observed_y, focal_x, focal_y,
        optical_center_x, optical_center_y, distortion_k1, distortion_k2,
        distortion_k3, distortion_p1, distortion_p2, point);

    problem.AddResidualBlock(cost_function, NULL, result_pose);      
  }

  ceres::Solver::Options options;
  ceres::Solver::Summary summary;
  options.linear_solver_type = ceres::DENSE_SCHUR;
  options.max_num_iterations = 9001;  

  if (output_results_)
  {
    options.minimizer_progress_to_stdout = true;
  }

  ceres::Solve(options, &problem, &summary);

  if (output_results_)
  {
    // Leaving this commented out for now, don't want to spam the terminal.
    // std::cout << sumary_.FullReport() << '\n';
  }

  if (summary.termination_type != ceres::NO_CONVERGENCE)
  {
    result = Pose6D(result_pose[3], result_pose[4], result_pose[5],
      result_pose[0], result_pose[1], result_pose[2]);
    return true;
  }
  else
  {
    std::cerr << "Failed to find a pose from the target, did not converge" << '\n';
    return false;
  }
  return false;
}

void CameraOnWristIntrinsic::displayCovariance(void)
{
  CovarianceRequest intrinsic_params_request;
  intrinsic_params_request.request_type = CovarianceRequestType::IntrinsicParams;
  intrinsic_params_request.object_name = "Intrinsics";

  CovarianceRequest target_pose_params_request;
  target_pose_params_request.request_type = CovarianceRequestType::TargetPoseParams;
  target_pose_params_request.object_name = "Target Pose";

  std::vector<CovarianceRequest> covariance_request;
  covariance_request.push_back(intrinsic_params_request);
  covariance_request.push_back(target_pose_params_request);

  this->computeCovariance(covariance_request, 0, 
    result_.target_to_camera, result_.intrinsics);   
}
} // namespace industrial_calibration_libs
