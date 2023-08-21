#pragma once

#include <open3d/Open3D.h>

#include "open3d/visualization/visualizer/GuiSettingsModel.h"
#include "open3d/visualization/visualizer/GuiSettingsView.h"
#include "open3d/visualization/visualizer/GuiWidgets.h"
#include "open3d/visualization/gui/FileDialog.h"

#include <open3d/visualization/gui/NumberEdit.h>

#include "utils.h"
#include "pointinfo_widget.h"

class MainWindow;

using namespace open3d::visualization;

enum MenuId {
    FILE_OPEN,
    FILE_EXPORT_RGB,
    FILE_QUIT,
    HELP_KEYS,
    HELP_CAMERA,
    HELP_ABOUT,
    HELP_CONTACT,
    HELP_INSTRUCTION_MANUAL,
    UNDO_TRANSFORMATION
};

//

struct GuiState {
    /// <summary>
    /// All point clouds currently loaded.
    /// </summary>
    std::vector<std::shared_ptr<Entry>> loaded_entries;

    /// <summary>
    /// The current point cloud selected by the user.
    /// Originates from `loaded_entries`, but a distinct copy.
    /// </summary>
    std::shared_ptr<Entry> current_entry;

    /// <summary>
    /// Accessing `loaded_entries` with this index results in the point cloud in `current_entry`
    /// (sans possible transformations).
    /// </summary>
    int entry_index;

    /// <summary>
    /// This widget contains the tools to manipulate point clouds.
    /// </summary>
    std::shared_ptr<PointInfo> point_info;

    /// <summary>
    /// This widget is used to render the clouds.
    /// </summary>
    std::shared_ptr<gui::SceneWidget> scene_wgt;

    std::shared_ptr<gui::VGrid> help_keys;
    std::shared_ptr<gui::VGrid> help_camera;

    int app_menu_custom_items_index_ = -1;
    std::shared_ptr<gui::Menu> app_menu;

    GuiSettingsModel model; // Contains default values

    /// <summary>
    /// Material used to present point clouds.
    /// </summary>
    rendering::MaterialRecord standard_material;

    MainWindow* window_ptr;

public:
    GuiState(MainWindow* window);

    void init_menu();
    void init_scene();
    void init_point_info();
    void init_materials();
    void init_lighting(const GuiSettingsModel::LightingProfile& lighting);
    void colorize_current_entry();

    void add_entry(const std::string& path, std::function<void(double)> update_progress, gui::Window* window);

    /// <summary>
    /// Re-renders existing point clouds.
    /// </summary>
    /// <param name="only_update_selected">Settings this value to true only re-renders the cloud in current_entry.
    /// Doing so may heavily increase performance.</param>
    /// <param name="keep_camera">Settings this value to true ensures that the camera stays at its current location.</param>
    void set_scene(bool only_update_selected, bool keep_camera);
};
