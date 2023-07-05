#include <data.h>
#include <main_window.h>
#include <gui_state.h>
#include <icp.h>

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
    file_menu->AddItem("Undo", UNDO_TRANSFORMATION);
    file_menu->AddSeparator();
#if defined(WIN32)
    file_menu->AddItem("Exit", FILE_QUIT);
#endif
    menu->AddMenu("File", file_menu);

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
    scene_wgt->EnableSceneCaching(true);
    scene_wgt->SetSunInteractorEnabled(false);
    scene_wgt->SetOnCameraChanged([this](rendering::Camera* cam) {
        auto render_scene = scene_wgt->GetScene()->GetScene();
        render_scene->SetSunLightDirection(cam->GetForwardVector());
        });
    auto* render_scene = scene_wgt->GetScene()->GetScene();
    render_scene->SetSunLightDirection(scene_wgt->GetScene()->GetCamera()->GetForwardVector());
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
            /// Making a distinct copy keeps the original cloud intact, allowing
            /// free manipulation.
            this->current_entry = std::make_shared<Entry>(*this->loaded_entries.at(index));
            this->colorize_current_entry();
            this->entry_index = index;
            this->point_info->SetName(this->current_entry->name.c_str());
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
                this->colorize_current_entry();
                this->entry_index = index;

                this->point_info->entries->SetSelectedIndex(index);
                this->point_info->SetName(this->current_entry->name.c_str());
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
            only_update_selected = true;

            if (loaded_entries.size() < 2) {
                this->window_ptr->ShowMessageBox("", "Es m\xC3\xBCssen mindestens zwei Punktewolken geladen sein, damit diese Funktion verf\xC3\xBCgbar ist.");
                return;
            }

            auto entries = std::make_shared<gui::Combobox>();
            for (int i = 0; i < loaded_entries.size(); i++) {
                if (i != entry_index) {
                    entries->AddItem(loaded_entries.at(i)->name.c_str());
                }
            }
            entries->SetSelectedIndex(0);

            auto ok = std::make_shared<gui::Button>("OK");
            ok->SetOnClicked([this, entries]() {
                int i = entries->GetSelectedIndex();
                if (i >= this->entry_index) {
                    i += 1;
                }

                Eigen::Matrix4d matrix = iterative_closest_point(this->current_entry->get_transformed().points_, this->loaded_entries.at(i)->get_transformed().points_);

                this->loaded_entries.at(entry_index)->do_transform(matrix);
                this->current_entry = std::make_shared<Entry>(*this->loaded_entries.at(this->entry_index));
                this->colorize_current_entry();

                this->window_ptr->CloseDialog();
                });

            auto cancel = std::make_shared<gui::Button>("Abbrechen");
            cancel->SetOnClicked([this]() { this->window_ptr->CloseDialog(); });

            auto layout = std::make_shared<gui::Vert>(0, gui::Margins(this->window_ptr->GetTheme().font_size));
            layout->AddChild(gui::Horiz::MakeCentered(entries));
            layout->AddFixed(this->window_ptr->GetTheme().font_size);

            auto buttons = std::make_shared<gui::Horiz>(0, this->window_ptr->GetTheme().font_size);
            buttons->AddChild(ok);
            buttons->AddFixed(this->window_ptr->GetTheme().font_size);
            buttons->AddChild(cancel);

            layout->AddChild(buttons);

            auto dialog = std::make_shared<gui::Dialog>("Ann\xC3\xB4hern");
            dialog->AddChild(layout);

            this->window_ptr->ShowDialog(dialog);
            break;
        }
        case MERGE_CLICKED: {
            only_update_selected = true;

            if (loaded_entries.size() < 2) {
                this->window_ptr->ShowMessageBox("", "Es m\xC3\xBCssen mindestens zwei Punktewolken geladen sein, damit diese Funktion verf\xC3\xBCgbar ist.");
                return;
            }

            auto entries = std::make_shared<gui::Combobox>();
            for (int i = 0; i < loaded_entries.size(); i++) {
                if (i != entry_index) {
                    entries->AddItem(loaded_entries.at(i)->name.c_str());
                }
            }
            entries->SetSelectedIndex(0);

            auto ok = std::make_shared<gui::Button>("OK");
            ok->SetOnClicked([this, entries]() {
                int i = entries->GetSelectedIndex();
                if (i >= this->entry_index) {
                    i += 1;
                }

                // Do not use current_entry, since it is recolored.
                auto& cloud_1 = this->loaded_entries.at(entry_index)->get_transformed();
                auto& cloud_2 = this->loaded_entries.at(i)->get_transformed();

                open3d::geometry::PointCloud cloud = open3d::geometry::PointCloud(cloud_1);
                cloud += cloud_2;

                std::shared_ptr<Entry> entry = std::make_shared<Entry>(cloud);

                this->loaded_entries.push_back(entry);
                this->point_info->entries->AddItem(entry->path.c_str());
                this->set_scene(false, true);

                this->window_ptr->CloseDialog();
                });

            auto cancel = std::make_shared<gui::Button>("Abbrechen");
            cancel->SetOnClicked([this]() { this->window_ptr->CloseDialog(); });

            auto layout = std::make_shared<gui::Vert>(0, gui::Margins(this->window_ptr->GetTheme().font_size));
            layout->AddChild(gui::Horiz::MakeCentered(entries));
            layout->AddFixed(this->window_ptr->GetTheme().font_size);

            auto buttons = std::make_shared<gui::Horiz>(0, this->window_ptr->GetTheme().font_size);
            buttons->AddChild(ok);
            buttons->AddFixed(this->window_ptr->GetTheme().font_size);
            buttons->AddChild(cancel);

            layout->AddChild(gui::Horiz::MakeCentered(buttons));

            auto dialog = std::make_shared<gui::Dialog>("Verschmelzen");
            dialog->AddChild(layout);

            this->window_ptr->ShowDialog(dialog);
            break;
        }
        case READ_MATRIX_CLICKED: {
            only_update_selected = true;

            auto grid = std::make_shared<gui::VGrid>(4, 0, gui::Margins(this->window_ptr->GetTheme().font_size));
            auto label = std::make_shared<gui::Label>("Determinante: 1.0");

            std::vector<std::shared_ptr<gui::NumberEdit>> editors;

            for (int i = 0; i < 12; i++) {
                editors.push_back(std::move(std::make_shared<gui::NumberEdit>(gui::NumberEdit::Type::DOUBLE)));
                auto& edit = editors.at(i);

                if (i == 0 || i == 5 || i == 10) {
                    edit->SetValue(1.0);
                }

                edit->SetDecimalPrecision(8);
                edit->SetOnValueChanged([&editors, label](double) {
                    Eigen::Matrix4d m = Eigen::Matrix4d::Zero();

                    for (int i = 0; i < 12; i++) {
                        m(i) = editors[i]->GetDoubleValue();
                    }

                    m(3, 3) = 1.0;

                    std::string str = "Determinante:" + std::to_string(m.determinant());
                    label->SetText(str.c_str());
                    });

                grid->AddChild(edit);
            }

            grid->AddChild(std::make_shared<gui::Label>("0.0"));
            grid->AddChild(std::make_shared<gui::Label>("0.0"));
            grid->AddChild(std::make_shared<gui::Label>("0.0"));
            grid->AddChild(std::make_shared<gui::Label>("1.0"));

            auto cancel = std::make_shared<gui::Button>("Abbrechen");
            cancel->SetOnClicked([this]() { this->window_ptr->CloseDialog(); });

            auto ok = std::make_shared<gui::Button>("OK");
            ok->SetOnClicked([this, &editors]() {
                Eigen::Matrix4d m = Eigen::Matrix4d::Zero();

                for (int i = 0; i < 12; i++) {
                    m(i) = editors[i]->GetDoubleValue();
                }

                m(3, 3) = 1.0;

                this->current_entry->do_transform(m);
                this->window_ptr->CloseDialog();
                });

            auto buttons = std::make_shared<gui::Horiz>(0, this->window_ptr->GetTheme().font_size);
            buttons->AddChild(ok);
            buttons->AddFixed(this->window_ptr->GetTheme().font_size);
            buttons->AddChild(cancel);

            auto layout = std::make_shared<gui::Vert>(0, gui::Margins(this->window_ptr->GetTheme().font_size));

            layout->AddChild(gui::Horiz::MakeCentered(grid));
            layout->AddChild(gui::Horiz::MakeCentered(label));
            layout->AddChild(gui::Horiz::MakeCentered(buttons));


            auto dialog = std::make_shared<gui::Dialog>("Matrix");
            dialog->AddChild(layout);

            this->window_ptr->ShowDialog(dialog);

            open3d::utility::LogInfo("ShowDialog call over");

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
            this->colorize_current_entry();
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
            this->colorize_current_entry();
            this->point_info->ResetSliders();
            only_update_selected = true;
            break;
        }
        case NAME_CHANGED: {
            if (this->loaded_entries.size() == 0) {
                break;
            }
            this->current_entry->name = std::string(event_.name);
            this->loaded_entries.at(this->entry_index)->name = event_.name;
            this->point_info->SetName(event_.name.c_str());
            return;
        }
        }

        this->set_scene(only_update_selected, true);
        });
}

GuiState::GuiState(MainWindow* window) : window_ptr(window) {
    loaded_entries = {};
    current_entry = std::make_shared<Entry>("empty");
    entry_index = -1;

    if (!gui::Application::GetInstance().GetMenubar()) {
        init_menu();
        gui::Application::GetInstance().SetMenubar(app_menu);
    }
    init_scene();
    window_ptr->AddChild(scene_wgt);

    init_materials();

    init_point_info();
    window_ptr->AddChild(point_info);

    // Apply model settings (which should be defaults) to the rendering entities
    auto scene = scene_wgt->GetScene();
    scene->ShowSkybox(false);
    scene->ShowAxes(true);
    scene->ShowGroundPlane(false, rendering::Scene::GroundPlane::XZ);
    scene_wgt->GetRenderView()->SetMode(rendering::View::Mode::Color);
    auto& lighting = model.GetLighting();
    init_lighting(lighting);
    // Make sure scene redraws once changes have been applied
    scene_wgt->ForceRedraw();

    // Other items
    help_keys = CreateHelpDisplay(window_ptr);
    help_keys->SetVisible(false);
    window_ptr->AddChild(help_keys);
    help_camera = CreateCameraDisplay(window_ptr);
    help_camera->SetVisible(false);
    window_ptr->AddChild(help_camera);
}

void GuiState::init_materials() {
    highlight_material.shader = "defaultLit";
    standard_material.shader = "defaultUnlit";

    auto& materials = model.GetCurrentMaterials();

    highlight_material.base_color.x() = materials.lit.base_color.x();
    highlight_material.base_color.y() = materials.lit.base_color.y();
    highlight_material.base_color.z() = materials.lit.base_color.z();
    highlight_material.point_size = materials.point_size;
    highlight_material.base_metallic = materials.lit.metallic;
    highlight_material.base_roughness = materials.lit.roughness;
    highlight_material.base_reflectance = materials.lit.reflectance;
    highlight_material.base_clearcoat = materials.lit.clear_coat;
    highlight_material.base_clearcoat_roughness = materials.lit.clear_coat_roughness;
    highlight_material.base_anisotropy = materials.lit.anisotropy;

    standard_material.base_color.x() = materials.unlit.base_color.x();
    standard_material.base_color.y() = materials.unlit.base_color.y();
    standard_material.base_color.z() = materials.unlit.base_color.z();
    standard_material.point_size = materials.point_size;

}

void GuiState::init_lighting(const GuiSettingsModel::LightingProfile& lighting) {
    auto scene = scene_wgt->GetScene();
    auto* render_scene = scene->GetScene();

    scene_wgt->SetOnCameraChanged([this](rendering::Camera* cam) {
        auto render_scene = scene_wgt->GetScene()->GetScene();
        render_scene->SetSunLightDirection(cam->GetForwardVector());
        });

    render_scene->SetSunLightDirection(scene->GetCamera()->GetForwardVector());
    render_scene->EnableIndirectLight(lighting.ibl_enabled);
    render_scene->SetIndirectLightIntensity(float(lighting.ibl_intensity));
    render_scene->SetIndirectLightRotation(lighting.ibl_rotation);
    render_scene->SetSunLightColor(lighting.sun_color);
    render_scene->SetSunLightIntensity(float(lighting.sun_intensity));
    render_scene->SetSunLightDirection(lighting.sun_dir);
    render_scene->EnableSunLight(lighting.sun_enabled);
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
                    this->current_entry = std::make_shared<Entry>(*entry);
                    this->colorize_current_entry();
                    this->entry_index = 0;
                    this->point_info->entries->SetSelectedIndex(0);
                    this->point_info->SetName(this->current_entry->name.c_str());
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

            // Ignore the cloud in loaded_entries,
            // if it is used for current_entry.
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

void GuiState::colorize_current_entry() {
    auto& cloud = this->current_entry->get_transformed();
    cloud.colors_.clear();

    for (int i = 0; i < cloud.points_.size(); i++) {
        Eigen::Vector3d c(0.5, 0.8, 0.0);
        cloud.colors_.push_back(c);
    }
}
