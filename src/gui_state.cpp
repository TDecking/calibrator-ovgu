#include <data.h>
#include <main_window.h>
#include <gui_state.h>

std::shared_ptr<gui::VGrid> CreateHelpDisplay(gui::Window* window) {
    auto& theme = window->GetTheme();

    gui::Margins margins(theme.font_size);
    auto layout = std::make_shared<gui::VGrid>(2, 0, margins);
    layout->SetBackgroundColor(gui::Color(0, 0, 0, 0.5));

    auto AddLabel = [layout](const char* text) {
        auto label = std::make_shared<gui::Label>(text);
        label->SetTextColor(gui::Color(1, 1, 1));
        layout->AddChild(label);
    };
    auto AddRow = [layout, &AddLabel](const char* left, const char* right) {
        AddLabel(left);
        AddLabel(right);
    };

    AddRow("Arcball mode", " ");
    AddRow("Left-drag", "Rotate camera");
    AddRow("Shift + left-drag", "Forward/backward");
    AddLabel("Ctrl + left-drag");
    AddLabel("Pan camera");
    AddLabel("Win + left-drag (up/down)  ");
    AddLabel("Rotate around forward axis");

    // GNOME3 uses Win/Meta as a shortcut to move windows around, so we
    // need another way to rotate around the forward axis.
    AddLabel("Ctrl + Shift + left-drag");
    AddLabel("Rotate around forward axis");
    AddLabel("Alt + left-drag");
    AddLabel("Rotate directional light");

    AddRow("Right-drag", "Pan camera");
    AddRow("Middle-drag", "Rotate directional light");
    AddRow("Wheel", "Forward/backward");
    AddRow("Shift + Wheel", "Change field of view");
    AddRow("", "");

    AddRow("Fly mode", " ");
    AddRow("Left-drag", "Rotate camera");
    AddLabel("Win + left-drag");
    AddLabel("Rotate around forward axis");
    AddRow("W", "Forward");
    AddRow("S", "Backward");
    AddRow("A", "Step left");
    AddRow("D", "Step right");
    AddRow("Q", "Step up");
    AddRow("Z", "Step down");
    AddRow("E", "Roll left");
    AddRow("R", "Roll right");
    AddRow("Up", "Look up");
    AddRow("Down", "Look down");
    AddRow("Left", "Look left");
    AddRow("Right", "Look right");

    return layout;
}

std::shared_ptr<gui::VGrid> CreateCameraDisplay(gui::Window* window) {
    auto& theme = window->GetTheme();

    gui::Margins margins(theme.font_size);
    auto layout = std::make_shared<gui::VGrid>(2, 0, margins);
    layout->SetBackgroundColor(gui::Color(0, 0, 0, 0.5));

    auto AddLabel = [layout](const char* text) {
        auto label = std::make_shared<gui::Label>(text);
        label->SetTextColor(gui::Color(1, 1, 1));
        layout->AddChild(label);
    };
    auto AddRow = [layout, &AddLabel](const char* left, const char* right) {
        AddLabel(left);
        AddLabel(right);
    };

    AddRow("Position:", "[0 0 0]");
    AddRow("Forward:", "[0 0 0]");
    AddRow("Left:", "[0 0 0]");
    AddRow("Up:", "[0 0 0]");

    return layout;
}

void GuiState::init_menu() {
    auto menu = std::make_shared<gui::Menu>();
    auto file_menu = std::make_shared<gui::Menu>();
    file_menu->AddItem("Open...", FILE_OPEN, gui::KEY_O);
    file_menu->AddItem("Export Current Image...", FILE_EXPORT_RGB);
    file_menu->AddSeparator();
#if defined(WIN32)
    file_menu->AddItem("Exit", FILE_QUIT);
#endif
    menu->AddMenu("File", file_menu);

    auto settings_menu = std::make_shared<gui::Menu>();
    settings_menu->AddItem("Lighting & Materials",
        SETTINGS_LIGHT_AND_MATERIALS);
    settings_menu->SetChecked(SETTINGS_LIGHT_AND_MATERIALS, true);
    menu->AddMenu("Settings", settings_menu);

    auto help_menu = std::make_shared<gui::Menu>();
    help_menu->AddItem("Show Controls", HELP_KEYS);
    help_menu->AddItem("Show Camera Info", HELP_CAMERA);
    help_menu->AddSeparator();
    help_menu->AddItem("About", HELP_ABOUT);
    help_menu->AddItem("Contact", HELP_CONTACT);
    menu->AddMenu("Help", help_menu);
    app_menu = menu;
}

void GuiState::init_scene() {
    scene_wgt = std::make_shared<gui::SceneWidget>();
    scene_wgt->SetScene(
        std::make_shared<rendering::Open3DScene>(window_ptr->GetRenderer()));
    scene_wgt->SetOnSunDirectionChanged(
        [this](const Eigen::Vector3f& new_dir) {
            auto lighting = settings.model.GetLighting();  // copy
            lighting.sun_dir = new_dir.normalized();
            settings.model.SetCustomLighting(lighting);
        });
    scene_wgt->EnableSceneCaching(true);
}

void GuiState::init_point_info() {
    auto& app = gui::Application::GetInstance();
    auto& theme = window_ptr->GetTheme();

    const float em = (float)theme.font_size;
    const int lm = int(std::ceil(0.5 * em));
    const int grid_spacing = int(std::ceil(0.25 * em));
    const gui::Margins base_margins(int(std::round(0.5 * lm)), lm, lm, lm);

    this->point_info = std::make_shared<PointInfo>(0, base_margins);

    this->point_info->SetEventHandler([this](PointInfoEvent& event_) {
        bool only_update_selected = false;

        switch (event_.type) {
        case INDEX_CHANGED: {
            int index = event_.entry_index;
            this->current_entry = std::make_shared<Entry>(*this->loaded_entries.at(index));
            this->entry_index = index;
            break;
        }
        case REMOVE_CLICKED: {
            int index = this->entry_index;

            const char* path = this->loaded_entries.at(index)->path.c_str();
            this->point_info->entries->RemoveItem(path);
            this->loaded_entries.erase(this->loaded_entries.begin() + index);

            index -= 1;
            if (index < 0 && this->loaded_entries.size() > 0) {
                index = 0;
            }

            if (index >= 0) {
                const Entry& e = *this->loaded_entries.at(index);
                this->current_entry = std::make_shared<Entry>(e);
                this->entry_index = index;

                this->point_info->entries->SetSelectedIndex(index);
            }
            else {
                this->loaded_entries = {};
                this->current_entry = std::make_shared<Entry>("empty");
                this->entry_index = -1;
            }

            this->point_info->ResetSliders();
            break;
        }
        case ICP_CLICKED: {
            break;
        }
        case MERGE_CLICKED: {
            break;
        }
        case READ_MATRIX_CLICKED: {
            break;
        }
        case SHOW_MATRIX_CLICKED: {
            auto matrix = current_entry->get_transformation();

            std::stringstream stream;
            stream << matrix;

            auto text_box = std::make_shared<gui::Label>();
            text_box->SetText(stream.str().c_str());

            auto ok = std::make_shared<gui::Button>("OK");
            ok->SetOnClicked([this]() { this->window_ptr->CloseDialog(); });

            auto layout = std::make_shared<gui::Vert>(0, gui::Margins(this->window_ptr->GetTheme().font_size));
            layout->AddChild(gui::Horiz::MakeCentered(text_box));
            layout->AddFixed(this->window_ptr->GetTheme().font_size);
            layout->AddChild(gui::Horiz::MakeCentered(ok));

            auto dialog = std::make_shared<gui::Dialog>("Matrix");
            dialog->AddChild(layout);

            this->window_ptr->ShowDialog(dialog);

            break;
        }
        case SLIDER_VALUE_CHANGED: {
            Eigen::Matrix4d t = make_matrix(event_.x_rotation, event_.y_rotation, event_.z_rotation, event_.x_translation, event_.y_translation, event_.z_translation);
            this->current_entry = std::make_shared<Entry>(*this->loaded_entries.at(this->entry_index));
            this->current_entry->do_transform(t);
            only_update_selected = true;
            break;
        }
        case SLIDER_MOUSE_RELEASE: {
            if (this->loaded_entries.size() == 0) {
                break;
            }
            Eigen::Matrix4d t = make_matrix(event_.x_rotation, event_.y_rotation, event_.z_rotation, event_.x_translation, event_.y_translation, event_.z_translation);
            this->loaded_entries.at(entry_index)->do_transform(t);
            this->current_entry = std::make_shared<Entry>(*this->loaded_entries.at(this->entry_index));
            this->point_info->ResetSliders();
            only_update_selected = true;
            break;
        }
        }

        this->set_scene(only_update_selected, true);
        });
}

void GuiState::init_settings() {
    auto& app = gui::Application::GetInstance();
    auto& theme = window_ptr->GetTheme();

    // Create light
    std::string resource_path = app.GetResourcePath();
    auto ibl_path = resource_path + "/default";
    auto* render_scene = scene_wgt->GetScene()->GetScene();
    render_scene->SetIndirectLight(ibl_path);

    // Create materials
    InitializeMaterials(window_ptr->GetRenderer(), resource_path);

    // Setup UI
    const auto em = theme.font_size;
    const int lm = int(std::ceil(0.5 * em));
    const int grid_spacing = int(std::ceil(0.25 * em));

    // Add settings widget
    const int separation_height = int(std::ceil(0.75 * em));
    // (we don't want as much left margin because the twisty arrow is the
    // only thing there, and visually it looks larger than the right.)
    const gui::Margins base_margins(int(std::round(0.5 * lm)), lm, lm, lm);
    settings.wgt_base = std::make_shared<gui::Vert>(0, base_margins);

    gui::Margins indent(em, 0, 0, 0);
    auto view_ctrls =
        std::make_shared<gui::CollapsableVert>("Mouse controls", 0, indent);

    // ... view manipulator buttons
    settings.wgt_mouse_arcball = std::make_shared<SmallToggleButton>("Arcball");
    settings.wgt_mouse_arcball->SetOn(true);
    settings.wgt_mouse_arcball->SetOnClicked([this]() {
        SetMouseControls(window_ptr,
            gui::SceneWidget::Controls::ROTATE_CAMERA);
        });
    settings.wgt_mouse_fly = std::make_shared<SmallToggleButton>("Fly");
    settings.wgt_mouse_fly->SetOnClicked([this]() {
        SetMouseControls(window_ptr, gui::SceneWidget::Controls::FLY);
        });
    settings.wgt_mouse_model = std::make_shared<SmallToggleButton>("Model");
    settings.wgt_mouse_model->SetOnClicked([this]() {
        SetMouseControls(window_ptr,
            gui::SceneWidget::Controls::ROTATE_MODEL);
        });
    settings.wgt_mouse_sun = std::make_shared<SmallToggleButton>("Sun");
    settings.wgt_mouse_sun->SetOnClicked([this]() {
        SetMouseControls(window_ptr, gui::SceneWidget::Controls::ROTATE_SUN);
        });
    settings.wgt_mouse_ibl = std::make_shared<SmallToggleButton>("Environment");
    settings.wgt_mouse_ibl->SetOnClicked([this]() {
        SetMouseControls(window_ptr, gui::SceneWidget::Controls::ROTATE_IBL);
        });

    auto reset_camera = std::make_shared<SmallButton>("Reset camera");
    reset_camera->SetOnClicked([this]() {
        scene_wgt->GoToCameraPreset(
            gui::SceneWidget::CameraPreset::PLUS_Z);
        });

    auto camera_controls1 = std::make_shared<gui::Horiz>(grid_spacing);
    camera_controls1->AddStretch();
    camera_controls1->AddChild(settings.wgt_mouse_arcball);
    camera_controls1->AddChild(settings.wgt_mouse_fly);
    camera_controls1->AddChild(settings.wgt_mouse_model);
    camera_controls1->AddStretch();
    auto camera_controls2 = std::make_shared<gui::Horiz>(grid_spacing);
    camera_controls2->AddStretch();
    camera_controls2->AddChild(settings.wgt_mouse_sun);
    camera_controls2->AddChild(settings.wgt_mouse_ibl);
    camera_controls2->AddStretch();
    view_ctrls->AddChild(camera_controls1);
    view_ctrls->AddFixed(int(std::ceil(0.25 * em)));
    view_ctrls->AddChild(camera_controls2);
    view_ctrls->AddFixed(separation_height);
    view_ctrls->AddChild(gui::Horiz::MakeCentered(reset_camera));
    settings.wgt_base->AddChild(view_ctrls);

    // ... lighting and materials
    settings.view = std::make_shared<GuiSettingsView>(
        settings.model, theme, resource_path, [this](const char* name) {
            // Do not use custom light maps
        });
    settings.model.SetOnChanged([this](bool material_type_changed) {
        settings.view->Update();
        this->UpdateFromModel(window_ptr->GetRenderer(), material_type_changed);
        });
    settings.wgt_base->AddChild(settings.view);
}


GuiState::GuiState(MainWindow* window) : window_ptr(window) {
    if (!gui::Application::GetInstance().GetMenubar()) {
        init_menu();
        gui::Application::GetInstance().SetMenubar(app_menu);
    }
    init_scene();
    window_ptr->AddChild(scene_wgt);

    init_settings();
    window_ptr->AddChild(settings.wgt_base);
    
    init_point_info();
    window_ptr->AddChild(point_info);

    // Apply model settings (which should be defaults) to the rendering entities
    UpdateFromModel(window_ptr->GetRenderer(), false);

    // Other items
    help_keys = CreateHelpDisplay(window_ptr);
    help_keys->SetVisible(false);
    window_ptr->AddChild(help_keys);
    help_camera = CreateCameraDisplay(window_ptr);
    help_camera->SetVisible(false);
    window_ptr->AddChild(help_camera);
}


void GuiState::add_entry(const std::string& path, std::function<void(double)> update_progress, gui::Window* window) {
    std::shared_ptr<Entry> entry = NULL;

    try {
        entry = std::make_shared<Entry>(path, update_progress);
    }
    catch (...) {
        entry.reset();
    }

    if (entry) {
        gui::Application::GetInstance().PostToMainThread(
            window, [this, window, entry, path]() {
                this->loaded_entries.push_back(entry);
                this->point_info->entries->AddItem(path.c_str());
                if (this->entry_index < 0) {
                    this->current_entry = entry;
                    this->entry_index = 0;
                    this->point_info->entries->SetSelectedIndex(0);
                }
                this->set_scene(false, false);
                window->CloseDialog();
            });
    }
    else {
        gui::Application::GetInstance().PostToMainThread(window, [this, window, path]() {
                window->CloseDialog();
                auto msg = std::string("Could not load '") + path + "'.";
                window->ShowMessageBox("Error", msg.c_str());
            });
    }
}

void GuiState::set_scene(bool only_update_selected, bool keep_camera) {
    auto scene3d = scene_wgt->GetScene();

    // Only update point cloud that got changed
    if (only_update_selected && entry_index >= 0) {
        scene3d->ShowAxes(true);
        scene3d->RemoveGeometry(current_entry->path);
        const open3d::geometry::PointCloud& cloud = current_entry->get_transformed();
        scene3d->AddGeometry(current_entry->path, &cloud, highlight_material);
    }
    else {
        scene3d->ClearGeometry();

        for (int i = 0; i < loaded_entries.size(); i++) {
            std::shared_ptr<Entry> entry = NULL;
            if (i != entry_index) {
                entry = loaded_entries.at(i);
            }
            else {
                entry = current_entry;
            }

            const open3d::geometry::PointCloud& cloud = entry->get_transformed();

            if (i != entry_index) {
                scene3d->AddGeometry(entry->path, &cloud, standard_material);
            }
            else {
                scene3d->AddGeometry(entry->path, &cloud, highlight_material);
            }
        }

        scene3d->ShowAxes(true);
    }

    auto& bounds = scene3d->GetBoundingBox();

    if (!keep_camera) {
        scene_wgt->SetupCamera(60.0, bounds, bounds.GetCenter().cast<float>());
    }

    // Make sure scene is redrawn
    scene_wgt->ForceRedraw();
}
