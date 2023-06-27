#pragma once

#include <open3d/Open3D.h>

#include "open3d/visualization/visualizer/GuiSettingsModel.h"
#include "open3d/visualization/visualizer/GuiSettingsView.h"
#include "open3d/visualization/visualizer/GuiWidgets.h"
#include "open3d/visualization/gui/FileDialog.h"

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

    std::shared_ptr<gui::SceneWidget> scene_wgt;
    std::shared_ptr<gui::VGrid> help_keys;
    std::shared_ptr<gui::VGrid> help_camera;

    int app_menu_custom_items_index_ = -1;
    std::shared_ptr<gui::Menu> app_menu;

    bool sun_follows_camera_ = false;
    bool basic_mode_enabled_ = false;

    struct Settings {
        rendering::MaterialRecord lit_material;
        rendering::MaterialRecord unlit_material;
        rendering::MaterialRecord normal_depth_material;

        GuiSettingsModel model;
        std::shared_ptr<gui::Vert> wgt_base;
        std::shared_ptr<gui::Button> wgt_mouse_arcball;
        std::shared_ptr<gui::Button> wgt_mouse_fly;
        std::shared_ptr<gui::Button> wgt_mouse_model;
        std::shared_ptr<gui::Button> wgt_mouse_sun;
        std::shared_ptr<gui::Button> wgt_mouse_ibl;
        std::shared_ptr<GuiSettingsView> view;
    } settings;

    MainWindow* window_ptr;

public:
    void init_menu();
    void init_scene();
    void init_settings();

    GuiState(MainWindow* window);

    void InitializeMaterials(rendering::Renderer& renderer,
        const std::string& resource_path) {
        settings.lit_material.shader = "defaultLit";
        settings.unlit_material.shader = "defaultUnlit";

        auto& defaults = settings.model.GetCurrentMaterials();

        UpdateMaterials(renderer, defaults);
    }

    void SetMaterialsToDefault() {
        settings.view->ShowFileMaterialEntry(false);
        settings.model.SetMaterialsToDefault();
        settings.view->EnableEstimateNormals(false);
        // model's OnChanged callback will get called (if set), which will
        // update everything.
    }

    void SetMouseControls(gui::Window* window,
        gui::SceneWidget::Controls mode) {
        using Controls = gui::SceneWidget::Controls;
        scene_wgt->SetViewControls(mode);
        window->SetFocusWidget(scene_wgt.get());
        settings.wgt_mouse_arcball->SetOn(mode == Controls::ROTATE_CAMERA);
        settings.wgt_mouse_fly->SetOn(mode == Controls::FLY);
        settings.wgt_mouse_model->SetOn(mode == Controls::ROTATE_MODEL);
        settings.wgt_mouse_sun->SetOn(mode == Controls::ROTATE_SUN);
        settings.wgt_mouse_ibl->SetOn(mode == Controls::ROTATE_IBL);
    }

    void ModifyMaterialForBasicMode(rendering::MaterialRecord& basic_mat) {
        // Set parameters for 'simple' rendering
        basic_mat.base_color = { 1.f, 1.f, 1.f, 1.f };
        basic_mat.base_metallic = 0.f;
        basic_mat.base_roughness = 0.5f;
        basic_mat.base_reflectance = 0.8f;
        basic_mat.base_clearcoat = 0.f;
        basic_mat.base_anisotropy = 0.f;
        basic_mat.albedo_img.reset();
        basic_mat.normal_img.reset();
        basic_mat.ao_img.reset();
        basic_mat.metallic_img.reset();
        basic_mat.roughness_img.reset();
        basic_mat.reflectance_img.reset();
        basic_mat.sRGB_color = false;
        basic_mat.sRGB_vertex_color = false;
    }

    void SetBasicMode(bool enable) {
        auto o3dscene = scene_wgt->GetScene();
        auto view = o3dscene->GetView();
        auto low_scene = o3dscene->GetScene();

        // Set lighting environment for inspection
        if (enable) {
            // Set lighting environment for inspection
            o3dscene->SetBackground({ 1.f, 1.f, 1.f, 1.f });
            low_scene->ShowSkybox(false);
            view->SetShadowing(false, rendering::View::ShadowType::kPCF);
            view->SetPostProcessing(false);
        }
        else {
            view->SetPostProcessing(true);
            view->SetShadowing(true, rendering::View::ShadowType::kPCF);
        }
    }

    void UpdateFromModel(rendering::Renderer& renderer, bool material_changed) {
        auto o3dscene = scene_wgt->GetScene();

        if (settings.model.GetShowSkybox()) {
            o3dscene->ShowSkybox(true);
        }
        else {
            o3dscene->ShowSkybox(false);
        }

        o3dscene->ShowAxes(settings.model.GetShowAxes());
        o3dscene->ShowGroundPlane(settings.model.GetShowGround(),
            rendering::Scene::GroundPlane::XZ);

        if (settings.model.GetBasicMode() != basic_mode_enabled_) {
            basic_mode_enabled_ = settings.model.GetBasicMode();
            SetBasicMode(basic_mode_enabled_);
        }

        UpdateLighting(renderer, settings.model.GetLighting());

        // Make sure scene redraws once changes have been applied
        scene_wgt->ForceRedraw();

        // Bail early if there were no material property changes
        if (!material_changed) return;

        // update matieral
        auto& current_materials = settings.model.GetCurrentMaterials();
        UpdateMaterials(renderer, current_materials);
        UpdateSceneMaterial();

        scene_wgt->GetRenderView()->SetMode(rendering::View::Mode::Color);

        // Make sure scene redraws once material changes have been applied
        scene_wgt->ForceRedraw();
    }

    void UpdateLighting(rendering::Renderer& renderer,
        const GuiSettingsModel::LightingProfile& lighting) {
        auto scene = scene_wgt->GetScene();
        auto* render_scene = scene->GetScene();
        if (sun_follows_camera_ != settings.model.GetSunFollowsCamera()) {
            sun_follows_camera_ = settings.model.GetSunFollowsCamera();
            if (sun_follows_camera_) {
                scene_wgt->SetOnCameraChanged([this](rendering::Camera* cam) {
                    auto render_scene = scene_wgt->GetScene()->GetScene();
                    render_scene->SetSunLightDirection(cam->GetForwardVector());
                    });
                render_scene->SetSunLightDirection(
                    scene->GetCamera()->GetForwardVector());
                settings.wgt_mouse_sun->SetEnabled(false);
                scene_wgt->SetSunInteractorEnabled(false);
            }
            else {
                scene_wgt->SetOnCameraChanged(
                    std::function<void(rendering::Camera*)>());
                settings.wgt_mouse_sun->SetEnabled(true);
                scene_wgt->SetSunInteractorEnabled(true);
            }
        }

        render_scene->EnableIndirectLight(lighting.ibl_enabled);
        render_scene->SetIndirectLightIntensity(float(lighting.ibl_intensity));
        render_scene->SetIndirectLightRotation(lighting.ibl_rotation);
        render_scene->SetSunLightColor(lighting.sun_color);
        render_scene->SetSunLightIntensity(float(lighting.sun_intensity));
        if (!sun_follows_camera_) {
            render_scene->SetSunLightDirection(lighting.sun_dir);
        }
        else {
            render_scene->SetSunLightDirection(
                scene->GetCamera()->GetForwardVector());
        }
        render_scene->EnableSunLight(lighting.sun_enabled);
    }

    void UpdateSceneMaterial() {
        open3d::visualization::rendering::MaterialRecord material;
        if (settings.model.GetMaterialType() == GuiSettingsModel::MaterialType::LIT) {
            material = settings.lit_material;
        }
        else {
            material = settings.unlit_material;
        }

        if (basic_mode_enabled_) {
            rendering::MaterialRecord basic_mat(material);
            ModifyMaterialForBasicMode(basic_mat);
            scene_wgt->GetScene()->UpdateMaterial(basic_mat);
        }
        else {
            scene_wgt->GetScene()->UpdateMaterial(material);
        }
    }

    void UpdateMaterials(rendering::Renderer& renderer,
        const GuiSettingsModel::Materials& materials) {
        auto& lit = settings.lit_material;
        auto& unlit = settings.unlit_material;
        auto& normal_depth = settings.normal_depth_material;

        // Update lit from GUI
        lit.base_color.x() = materials.lit.base_color.x();
        lit.base_color.y() = materials.lit.base_color.y();
        lit.base_color.z() = materials.lit.base_color.z();
        lit.point_size = materials.point_size;
        lit.base_metallic = materials.lit.metallic;
        lit.base_roughness = materials.lit.roughness;
        lit.base_reflectance = materials.lit.reflectance;
        lit.base_clearcoat = materials.lit.clear_coat;
        lit.base_clearcoat_roughness = materials.lit.clear_coat_roughness;
        lit.base_anisotropy = materials.lit.anisotropy;

        // Update unlit from GUI
        unlit.base_color.x() = materials.unlit.base_color.x();
        unlit.base_color.y() = materials.unlit.base_color.y();
        unlit.base_color.z() = materials.unlit.base_color.z();
        unlit.point_size = materials.point_size;

        // Update normal/depth from GUI
        normal_depth.point_size = materials.point_size;
    }

    void set_scene(bool only_update_selected, bool keep_camera);
};