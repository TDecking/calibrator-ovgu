#include <main_window.h>
#include <gui_state.h>

#include "open3d/visualization/visualizer/GuiSettingsModel.h"
#include "open3d/visualization/visualizer/GuiSettingsView.h"
#include "open3d/visualization/visualizer/GuiWidgets.h"
#include "open3d/visualization/gui/FileDialog.h"

using namespace open3d::visualization;


std::shared_ptr<gui::Dialog> CreateContactDialog(gui::Window* window) {
    auto& theme = window->GetTheme();
    auto em = theme.font_size;
    auto dlg = std::make_shared<gui::Dialog>("Contact Us");

    auto title = std::make_shared<gui::Label>("Contact Us");
    auto left_col = std::make_shared<gui::Label>(
        "Web site:\n"
        "Code:\n"
        "Mailing list:\n"
        "Discord channel:");
    auto right_col = std::make_shared<gui::Label>(
        "http://www.open3d.org\n"
        "http://github.org/isl-org/Open3D\n"
        "http://www.open3d.org/index.php/subscribe/\n"
        "https://discord.gg/D35BGvn");
    auto ok = std::make_shared<gui::Button>("OK");
    ok->SetOnClicked([window]() { window->CloseDialog(); });

    gui::Margins margins(em);
    auto layout = std::make_shared<gui::Vert>(0, margins);
    layout->AddChild(gui::Horiz::MakeCentered(title));
    layout->AddFixed(em);

    auto columns = std::make_shared<gui::Horiz>(em, gui::Margins());
    columns->AddChild(left_col);
    columns->AddChild(right_col);
    layout->AddChild(columns);

    layout->AddFixed(em);
    layout->AddChild(gui::Horiz::MakeCentered(ok));
    dlg->AddChild(layout);

    return dlg;
}

std::shared_ptr<gui::Dialog> CreateAboutDialog(gui::Window* window) {
    auto& theme = window->GetTheme();
    auto dlg = std::make_shared<gui::Dialog>("About");

    auto title = std::make_shared<gui::Label>(
        (std::string("Open3D ") + OPEN3D_VERSION).c_str());
    auto text = std::make_shared<gui::Label>(
        "The MIT License (MIT)\n"
        "Copyright (c) 2018-2023 www.open3d.org\n\n"

        "Permission is hereby granted, free of charge, to any person "
        "obtaining a copy of this software and associated documentation "
        "files (the \"Software\"), to deal in the Software without "
        "restriction, including without limitation the rights to use, "
        "copy, modify, merge, publish, distribute, sublicense, and/or "
        "sell copies of the Software, and to permit persons to whom "
        "the Software is furnished to do so, subject to the following "
        "conditions:\n\n"

        "The above copyright notice and this permission notice shall be "
        "included in all copies or substantial portions of the "
        "Software.\n\n"

        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, "
        "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES "
        "OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND "
        "NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT "
        "HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, "
        "WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING "
        "FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR "
        "OTHER DEALINGS IN THE SOFTWARE.");
    auto ok = std::make_shared<gui::Button>("OK");
    ok->SetOnClicked([window]() { window->CloseDialog(); });

    gui::Margins margins(theme.font_size);
    auto layout = std::make_shared<gui::Vert>(0, margins);
    layout->AddChild(gui::Horiz::MakeCentered(title));
    layout->AddFixed(theme.font_size);
    auto v = std::make_shared<gui::ScrollableVert>(0);
    v->AddChild(text);
    layout->AddChild(v);
    layout->AddFixed(theme.font_size);
    layout->AddChild(gui::Horiz::MakeCentered(ok));
    dlg->AddChild(layout);

    return dlg;
}


bool ColorArrayIsUniform(const std::vector<Eigen::Vector3d>& colors) {
    static const double e = 1.0 / 255.0;
    static const double SQ_EPSILON = Eigen::Vector3d(e, e, e).squaredNorm();
    const auto& color = colors[0];

    for (const auto& c : colors) {
        if ((color - c).squaredNorm() > SQ_EPSILON) {
            return false;
        }
    }

    return true;
}

bool PointCloudHasUniformColor(const open3d::geometry::PointCloud& pcd) {
    if (!pcd.HasColors()) {
        return true;
    }
    return ColorArrayIsUniform(pcd.colors_);
};


const std::string MODEL_NAME = "__model__";
const std::string INSPECT_MODEL_NAME = "__inspect_model__";
const std::string WIREFRAME_NAME = "__wireframe_model__";


MainWindow::MainWindow(const std::string& title, int width, int height)
    : gui::Window(title, width, height)
{
    gui_state = std::make_unique<GuiState>((MainWindow*)this);
}

MainWindow::~MainWindow() {}

void MainWindow::SetTitle(const std::string& title) {
    Super::SetTitle(title.c_str());
}

void MainWindow::Layout(const gui::LayoutContext& context) {
    auto r = GetContentRect();
    const auto em = context.theme.font_size;
    gui_state->scene_wgt->SetFrame(r);

    // Draw help keys HUD in upper right
    const auto pref = gui_state->help_keys->CalcPreferredSize(
        context, gui::Widget::Constraints());
    gui_state->help_keys->SetFrame(gui::Rect(r.width - pref.width, r.y, pref.width, pref.height));
    gui_state->help_keys->Layout(context);

    // Draw camera HUD in lower left
    const auto prefcam = gui_state->help_camera->CalcPreferredSize(
        context, gui::Widget::Constraints());
    gui_state->help_camera->SetFrame(gui::Rect(0, r.height + r.y - prefcam.height,
        prefcam.width, prefcam.height));
    gui_state->help_camera->Layout(context);

    // Draw point info in the upper left
    static int MIN_WIDTH = 0;
    const auto prefpi = gui_state->point_info->CalcPreferredSize(
        context, gui::Widget::Constraints());
    MIN_WIDTH = MIN_WIDTH == 0 ? prefpi.width : MIN_WIDTH;
    gui_state->point_info->SetFrame(gui::Rect(0, r.y, MIN_WIDTH, prefpi.height));
    gui_state->point_info->Layout(context);

    Super::Layout(context);
}

void MainWindow::LoadCloud(const std::string& path) {
    auto progressbar = std::make_shared<gui::ProgressBar>();
    gui::Application::GetInstance().PostToMainThread(this, [this, path,
        progressbar]() {
            auto& theme = GetTheme();
            auto loading_dlg = std::make_shared<gui::Dialog>("Loading");
            auto vert =
                std::make_shared<gui::Vert>(0, gui::Margins(theme.font_size));
            auto loading_text = std::string("Loading ") + path;
            vert->AddChild(std::make_shared<gui::Label>(loading_text.c_str()));
            vert->AddFixed(theme.font_size);
            vert->AddChild(progressbar);
            loading_dlg->AddChild(vert);
            ShowDialog(loading_dlg);
        });

    gui::Application::GetInstance().RunInThread([this, path, progressbar]() {
        auto UpdateProgress = [this, progressbar](float value) {
            gui::Application::GetInstance().PostToMainThread(
                this,
                [progressbar, value]() { progressbar->SetValue(value); });
        };
        this->gui_state->add_entry(path, UpdateProgress, this);
    });
}

void MainWindow::ExportCurrentImage(const std::string& path) {
    gui_state->scene_wgt->EnableSceneCaching(false);
    gui_state->scene_wgt->GetScene()->GetScene()->RenderToImage(
        [this, path](std::shared_ptr<open3d::geometry::Image> image) mutable {
            if (!open3d::io::WriteImage(path, *image)) {
                this->ShowMessageBox(
                    "Error", (std::string("Could not write image to ") +
                        path + ".")
                    .c_str());
            }
            gui_state->scene_wgt->EnableSceneCaching(true);
        });
}

void MainWindow::OnMenuItemSelected(gui::Menu::ItemId item_id) {
    auto menu_id = MenuId(item_id);
    switch (menu_id) {
    case FILE_OPEN: {
        auto dlg = std::make_shared<gui::FileDialog>(
            gui::FileDialog::Mode::OPEN, "Open Geometry", GetTheme());
        dlg->AddFilter(".ply .stl .fbx .obj .off .gltf .glb",
            "Triangle mesh files (.ply, .stl, .fbx, .obj, .off, "
            ".gltf, .glb)");
        dlg->AddFilter(".xyz .xyzn .xyzrgb .ply .pcd .pts",
            "Point cloud files (.xyz, .xyzn, .xyzrgb, .ply, "
            ".pcd, .pts)");
        dlg->AddFilter(".ply", "Polygon files (.ply)");
        dlg->AddFilter(".stl", "Stereolithography files (.stl)");
        dlg->AddFilter(".fbx", "Autodesk Filmbox files (.fbx)");
        dlg->AddFilter(".obj", "Wavefront OBJ files (.obj)");
        dlg->AddFilter(".off", "Object file format (.off)");
        dlg->AddFilter(".gltf", "OpenGL transfer files (.gltf)");
        dlg->AddFilter(".glb", "OpenGL binary transfer files (.glb)");
        dlg->AddFilter(".xyz", "ASCII point cloud files (.xyz)");
        dlg->AddFilter(".xyzn", "ASCII point cloud with normals (.xyzn)");
        dlg->AddFilter(".xyzrgb",
            "ASCII point cloud files with colors (.xyzrgb)");
        dlg->AddFilter(".pcd", "Point Cloud Data files (.pcd)");
        dlg->AddFilter(".pts", "3D Points files (.pts)");
        dlg->AddFilter("", "All files");
        dlg->SetOnCancel([this]() { this->CloseDialog(); });
        dlg->SetOnDone([this](const char* path) {
            this->CloseDialog();
            OnDragDropped(path);
            });
        ShowDialog(dlg);
        break;
    }
    case FILE_EXPORT_RGB: {
        auto dlg = std::make_shared<gui::FileDialog>(
            gui::FileDialog::Mode::SAVE, "Save File", GetTheme());
        dlg->AddFilter(".png", "PNG images (.png)");
        dlg->AddFilter("", "All files");
        dlg->SetOnCancel([this]() { this->CloseDialog(); });
        dlg->SetOnDone([this](const char* path) {
            this->CloseDialog();
            this->ExportCurrentImage(path);
            });
        ShowDialog(dlg);
        break;
    }
    case FILE_QUIT:
        gui::Application::GetInstance().Quit();
        break;
    case HELP_KEYS: {
        bool is_visible = !gui_state->help_keys->IsVisible();
        gui_state->help_keys->SetVisible(is_visible);
        auto menubar = gui::Application::GetInstance().GetMenubar();
        menubar->SetChecked(HELP_KEYS, is_visible);
        break;
    }
    case HELP_CAMERA: {
        bool is_visible = !gui_state->help_camera->IsVisible();
        gui_state->help_camera->SetVisible(is_visible);
        auto menubar = gui::Application::GetInstance().GetMenubar();
        menubar->SetChecked(HELP_CAMERA, is_visible);
        if (is_visible) {
            gui_state->scene_wgt->SetOnCameraChanged([this](rendering::Camera
                * cam) {
                    auto children = this->gui_state->help_camera->GetChildren();
                    auto set_text = [](const Eigen::Vector3f& v,
                        std::shared_ptr<gui::Widget> label) {
                            auto l = std::dynamic_pointer_cast<gui::Label>(label);
                            l->SetText(fmt::format("[{:.2f} {:.2f} "
                                "{:.2f}]",
                                v.x(), v.y(), v.z())
                                .c_str());
                    };
                    set_text(cam->GetPosition(), children[1]);
                    set_text(cam->GetForwardVector(), children[3]);
                    set_text(cam->GetLeftVector(), children[5]);
                    set_text(cam->GetUpVector(), children[7]);
                    this->SetNeedsLayout();
                });
        }
        else {
            gui_state->scene_wgt->SetOnCameraChanged(
                std::function<void(rendering::Camera*)>());
        }
        break;
    }
    case HELP_ABOUT: {
        auto dlg = CreateAboutDialog(this);
        ShowDialog(dlg);
        break;
    }
    case HELP_CONTACT: {
        auto dlg = CreateContactDialog(this);
        ShowDialog(dlg);
        break;
    }
    case HELP_DEBUG: {
        break;
    }
    case UNDO_TRANSFORMATION: {
        this->gui_state->current_entry->undo_transform();
        this->gui_state->set_scene(true, true);
        break;
    }
    }
}

void MainWindow::OnDragDropped(const char* path) {
    this->LoadCloud(path);
}
