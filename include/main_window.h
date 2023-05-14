#pragma once

#include <open3d/Open3D.h>

#include <vector>


class MainWindow : public open3d::visualization::gui::Window {
    using Super = open3d::visualization::gui::Window;

public:
    MainWindow(const std::string& title, int width, int height);
    MainWindow(const std::vector<std::shared_ptr<const open3d::geometry::Geometry>>&
        geometries,
        const std::string& title,
        int width,
        int height,
        int left,
        int top);
    virtual ~MainWindow();

    void SetTitle(const std::string& title);
    void SetGeometry(std::shared_ptr<const open3d::geometry::Geometry> geometry);
    //void AddGeometry(std::shared_ptr<const open3d::geometry::Geometry> geometry);

    /// Loads asynchronously, will return immediately.
    void LoadGeometry(const std::string& path);

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