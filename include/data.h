#pragma once

#include <open3d/Open3D.h>
#include <vector>
#include <optional>

#include <functional>

class Entry {
    /// <summary>
    /// The file path from which the data originates.
    /// </summary>
    const std::string path;

    /// <summary>
    /// The original data.
    /// </summary>
    const open3d::geometry::PointCloud base;

    /// <summary>
    /// The transformed data. Equal 
    /// </summary>
    open3d::geometry::PointCloud transformed;

    /// <summary>
    /// A list of transformations that turns the point cloud stored
    /// in `base` into the point cloud stored in `transformed`.
    /// </summary>
    std::vector<Eigen::Matrix4d> transformations;

public:
    /// <summary>
    /// The name of the dataset.
    /// </summary>
    std::string name;

    /// <summary>
    /// Copy Contructor
    /// </summary>
    /// <param name="arg">Entry to be copied.</param>
    Entry(const Entry& arg);

    /// <summary>
    /// Contructor that creates an Entry by loading data from a path.
    /// </summary>
    /// <param name="path">The file path. Expected to be the ply file format.</param>
    Entry(const std::string path, std::function<void(double)> UpdateProgress);

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

