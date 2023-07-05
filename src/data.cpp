#include <data.h>


void Entry::recalculate_transform() {
    Eigen::Matrix4d t = get_transformation();

    for (int i = 0; i < base.points_.size(); i++) {
        Eigen::Vector3d p(base.points_.at(i));
        Eigen::Vector4d extended;
        extended << p, 1;
        Eigen::Vector4d result(t * extended);
        transformed.points_.at(i) = result.head<3>();
    }
}


Entry::Entry(const Entry& arg):
    path(arg.path),
    base(arg.base),
    transformed(arg.transformed),
    transformations(arg.transformations),
    name(arg.name)
{}


Eigen::Matrix4d Entry::get_transformation() {
    Eigen::Matrix4d result = Eigen::Matrix4d::Identity();

    for (const Eigen::Matrix4d& t : transformations) {
        // Be careful with operator ordering: transormations that get applied
        // first are on the right.
        result = t * result;
    }

    return result;
}


open3d::geometry::PointCloud load(std::string path, std::function<void(double)> UpdateProgress) {
    bool success = false;
    auto geometry_type = open3d::io::ReadFileGeometryType(path);

    open3d::geometry::PointCloud cloud;

    try {
        open3d::io::ReadPointCloudOption opt;
        opt.update_progress = [UpdateProgress](double percent) -> bool {
            UpdateProgress(float(percent / 100.0));
            return true;
        };
        success = open3d::io::ReadPointCloud(path, cloud, opt);
    }
    catch (...) {
        success = false;
    }

    if (!success) {
        throw NULL;
    }

    return cloud;
}

Entry::Entry(const std::string path, std::function<void(double)> UpdateProgress):
    path("cloud_" + std::to_string(id_counter++)),
    base(load(path, UpdateProgress)),
    transformed(base),
    transformations(),
    name(path) {
}

Entry::Entry(const open3d::geometry::PointCloud& cloud): path("cloud_" + std::to_string(id_counter++)), base(cloud), transformed(cloud), transformations(), name("") {
}

void Entry::do_transform(Eigen::Matrix4d transformation) {
    transformations.push_back(transformation);
    recalculate_transform();
}

std::optional<Eigen::Matrix4d> Entry::undo_transform() {
    if (transformations.size() == 0) {
        return std::nullopt;
    }

    auto result = std::optional{ Eigen::Matrix4d(transformations.back()) };
    transformations.pop_back();
    recalculate_transform();
    return result;
}

open3d::geometry::PointCloud& Entry::get_transformed() {
    return transformed;
}