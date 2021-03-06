#include <industrial_calibration_libs/cost_functions.h>
#include <industrial_calibration_libs/calibration.h>
#include <iomanip>

namespace industrial_calibration_libs
{
CalibrationJob::CalibrationJob(const ObservationData &observation_data,
  const Target &target) : observation_data_(observation_data), target_(target),
  output_results_(false) { }

bool CalibrationJob::checkObservations(void)
{
  // Checks if number of observations per image is consistant by setting
  // observations_per_image_ to the number of observations in the first index
  // of observation_data_.
  num_images_ = observation_data_.size();
  for (std::size_t i = 0; i < num_images_; i++)
  {
    if (i == 0)
    {
      observations_per_image_ = observation_data_[i].size();
    }
    else
    {
      if (observations_per_image_ != observation_data_[i].size()) {return false;}
    }
  }
  // Set total observations
  total_observations_ = num_images_ * observations_per_image_; 
  return true; 
}

bool CalibrationJob::computeCovariance(const std::vector<CovarianceRequest> &requests, double* extrinsics_result, double* target_pose_result,
  double* intrinsics_result)
{
  ceres::Covariance::Options covariance_options;
  covariance_options.algorithm_type = ceres::DENSE_SVD;
  ceres::Covariance covariance(covariance_options);

  std::vector<const double*> covariance_blocks;
  std::vector<int> block_sizes;
  std::vector<std::string> block_names;
  std::vector<std::pair<const double*, const double*>> covariance_pairs;

  for (auto &request : requests)
  {
    double* intrinsics;
    double* extrinsics;
    double* target_pose;

    switch (request.request_type)
    {
      case IntrinsicParams:
        intrinsics = intrinsics_result;
        covariance_blocks.push_back(intrinsics);
        block_sizes.push_back(9);
        block_names.push_back(request.object_name);
        break;

      case ExtrinsicParams:
        extrinsics = extrinsics_result;
        covariance_blocks.push_back(extrinsics);
        block_sizes.push_back(6);
        block_names.push_back(request.object_name);
        break;

      case TargetPoseParams:
        target_pose = target_pose_result;
        covariance_blocks.push_back(target_pose);
        block_sizes.push_back(6);
        block_names.push_back(request.object_name);
        break;

      default:
        return false;
        break;
    }
  }

  // Create pairs from every combination of blocks in request
  for (std::size_t i = 0; i < covariance_blocks.size(); i++)
  {
    for (std::size_t j = 0; j < covariance_blocks.size(); j++)
    {
      covariance_pairs.push_back(std::make_pair(covariance_blocks[i],
        covariance_blocks[j]));
    }
  }

  covariance.Compute(covariance_pairs, &problem_);

  if (output_results_)
  {
    std::cout << "Covariance Blocks: " << '\n';
    for (std::size_t i = 0; i < covariance_blocks.size(); i++)
    {
      for (std::size_t j = 0; j < covariance_blocks.size(); j++)
      {
        std::cout << "Covariance [" << block_names[i] << ", " 
          << block_names[j] << "]" << '\n';

        int N = block_sizes[i];
        int M = block_sizes[j];
        double* ij_cov_block = new double[N*M];

        covariance.GetCovarianceBlock(covariance_blocks[i], covariance_blocks[j],
          ij_cov_block);

        for (int q = 0; q < N; q++)
        {
          std::cout << "[";
          for (int k = 0; k < M; k++)
          {
            double sigma_i = sqrt(ij_cov_block[q*N+q]);
            double sigma_j = sqrt(ij_cov_block[k*N+k]);
            if (q == k)
            {
              if (sigma_i > 1.0 || sigma_i < -1.0)
              {
                std::cout << " " << std::right << std::setw(9) << std::scientific 
                  << std::setprecision(1) << sigma_i;              
              }
              else
              {
                std::cout << " " << std::right << std::setw(9) << std::fixed
                  << std::setprecision(5) << sigma_i;
              }
            }
            else
            {
              if (ij_cov_block[q*N + k]/(sigma_i * sigma_j) > 1.0 ||
                ij_cov_block[q*N + k]/(sigma_i * sigma_j) < -1.0)
              {
                std::cout << " " << std::right << std::setw(9) << std::scientific
                  << std::setprecision(1) << ij_cov_block[q*N + k]/(sigma_i * sigma_j);
              }
              else
              {
                std::cout << " " << std::right << std::setw(9) << std::fixed 
                  << std::setprecision(5) << ij_cov_block[q*N + k]/(sigma_i * sigma_j);
              }
            }
          }
          std::cout << "]" << '\n';
        }
        delete [] ij_cov_block;
      }
    }
  }
  return true;
}
} // namespace industrial_calibration_libs