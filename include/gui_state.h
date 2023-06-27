#pragma once

#include <open3d/Open3D.h>

#include "open3d/visualization/visualizer/GuiSettingsModel.h"
#include "open3d/visualization/visualizer/GuiSettingsView.h"
#include "open3d/visualization/visualizer/GuiWidgets.h"
#include "open3d/visualization/gui/FileDialog.h"

#include "utils.h"
#include "pointinfo_widget.h"

class MainWindow;

using namespace open3d::visualization;

enum MenuId {
    FILE_OPEN,
    FILE_EXPORT_RGB,
    FILE_QUIT,
    SETTINGS_LIGHT_AND_MATERIALS,
    HELP_KEYS,
    HELP_CAMERA,
    HELP_ABOUT,
    HELP_CONTACT,
    HELP_DEBUG
};

//

struct GuiState {
    std::vector<std::shared_ptr<Entry>> loaded_entries;
    std::shared_ptr<Entry> current_entry;
    int entry_index;

    std::shared_ptr<PointInfo> point_info;
    std::shared_ptr<gui::SceneWidget> scene_wgt;

    std::shared_ptr<gui::VGrid> help_keys;
    std::shared_ptr<gui::VGrid> help_camera;

    int app_menu_custom_items_index_ = -1;
    std::shared_ptr<gui::Menu> app_menu;

    GuiSettingsModel model; // Contains default values

    rendering::MaterialRecord standard_material;
    rendering::MaterialRecord highlight_material;

    MainWindow* window_ptr;

public:
    GuiState(MainWindow* window);

    void init_menu();
    void init_scene();
    void init_point_info();
    void init_materials();
    void init_lighting(const GuiSettingsModel::LightingProfile& lighting);

    void add_entry(const std::string& path, std::function<void(double)> update_progress, gui::Window* window);
    void set_scene(bool only_update_selected, bool keep_camera);
};