#pragma once

#include <open3d/Open3D.h>
#include <vector>

#include <data.h>


struct GuiState;

class MainWindow : public open3d::visualization::gui::Window {
    using Super = open3d::visualization::gui::Window;

public:
    MainWindow(const std::string& title, int width, int height);
    virtual ~MainWindow();

    void SetTitle(const std::string& title);

    /// Loads asynchronously, will return immediately.
    void LoadCloud(const std::string& path);

    void ExportCurrentImage(const std::string& path);

    void Layout(const open3d::visualization::gui::LayoutContext& context) override;

protected:
    void OnMenuItemSelected(open3d::visualization::gui::Menu::ItemId item_id) override;
    void OnDragDropped(const char* path) override;

private:
    std::unique_ptr<GuiState> gui_state;
};