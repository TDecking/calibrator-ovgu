#pragma once

#include <open3d/Open3D.h>
#include <vector>
#include <optional>

#include <functional>

#include <utils.h>

class Entry {
    /// <summary>
    /// The original data.
    /// </summary>
    const open3d::geometry::PointCloud base;

    /// <summary>
    /// The transformed data. Equal to `base` with `get_transformation()` applied
    /// </summary>
    open3d::geometry::PointCloud transformed;

    /// <summary>
    /// A list of transformations that turns the point cloud stored
    /// in `base` into the point cloud stored in `transformed`.
    /// </summary>
    std::vector<Eigen::Matrix4d> transformations;

public:
    /// <summary>
    /// Used to disambiguate entries for rendering.
    /// </summary>
    const std::string path;

    /// <summary>
    /// The name of the dataset.
    /// </summary>
    std::string name;

    /// <summary>
    /// Contructor that creates an Entry with zero points.
    /// </summary>
    inline Entry(const std::string& path_) : path("cloud_" + std::to_string(id_counter++)), name(path_), transformations(), base(), transformed() {}

    /// <summary>
    /// Copy Contructor
    /// </summary>
    /// <param name="arg">Entry to be copied.</param>
    Entry(const Entry& arg);

    /// <summary>
    /// Contructor that creates an Entry by loading data from a path.
    /// </summary>
    /// <param name="path">The file path. Expected to be the ply file format.</param>
    /// <param name="update_progress">Callback for loading progress.</param>
    Entry(const std::string path, std::function<void(double)> update_progress);

    /// <summary>
    /// Contructor that creates an Entry using a pre-existing cloud.
    /// </summary>
    /// <param name="cloud">Input cloud.</param>
    Entry(const open3d::geometry::PointCloud& cloud);

    /// <summary>
    /// Add a transformation to the transformation stack.
    /// Updates the transformed point cloud accordingly.
    /// </summary>
    /// <param name="transformation">The transformation to be performed</param>
    void do_transform(Eigen::Matrix4d transformation);

    /// <summary>
    /// Remove a transformation to the transformation stack, if possible.
    /// Updates the transformed point cloud accordingly.
    /// </summary>
    /// <returns>The removed transformation, if one was present.</returns>
    std::optional<Eigen::Matrix4d> undo_transform();

    /// <summary>
    /// Return a reference to the transformed data.
    /// </summary>
    /// <returns></returns>
    open3d::geometry::PointCloud& get_transformed();

    /// <summary>
    /// Returns the internal transformation.
    /// </summary>
    /// <returns></returns>
    Eigen::Matrix4d get_transformation();

private:
    /// <summary>
    /// Ensures that `transformed` is equal to `base` with the transformations in `transformations` applied.
    /// </summary>
    void recalculate_transform();
};

class Data {
public:
private:
    std::vector<std::shared_ptr<Entry>> loaded_clouds;
};

