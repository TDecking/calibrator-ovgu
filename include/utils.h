#pragma once

#include <open3d/Open3D.h>

static uint64_t id_counter = 0;

Eigen::Matrix4d make_matrix(double x_rotation, double y_rotation, double z_rotation, double x_translation, double y_translation, double z_translation);
