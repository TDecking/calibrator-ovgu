#pragma once

#include <open3d/Open3D.h>

#include <vector>


class MainWindow : public open3d::visualization::gui::Window {
    using Super = open3d::visualization::gui::Window;

public:
    MainWindow(const std::string& title, int width, int height);
    MainWindow(const std::vector<std::shared_ptr<const open3d::geometry::PointCloud>>&
        point_clouds,
        const std::string& title,
        int width,
        int height,
        int left,
        int top);
    virtual ~MainWindow();

    void SetTitle(const std::string& title);
    void MainWindow::SetClouds(const std::vector<std::shared_ptr<const open3d::geometry::PointCloud>>& point_clouds);

    /// Loads asynchronously, will return immediately.
    void LoadCloud(const std::string& path);

    void ExportCurrentImage(const std::string& path);

    void Layout(const open3d::visualization::gui::LayoutContext& context) override;

protected:
    void OnMenuItemSelected(open3d::visualization::gui::Menu::ItemId item_id) override;
    void OnDragDropped(const char* path) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    void Init();
};