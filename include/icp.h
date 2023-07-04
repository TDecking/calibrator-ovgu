#include <open3d/Open3D.h>

Eigen::Matrix4d iterative_closest_point(const std::vector<Eigen::Vector3d>& src, const std::vector<Eigen::Vector3d>& target);