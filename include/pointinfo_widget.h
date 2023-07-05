#pragma once

#include <functional>

#include <open3d/Open3D.h>

#include <open3d/visualization/gui/ListView.h>

using namespace open3d::visualization;


class PointInfo;

class MouseEventSlider : public gui::Slider {
public:
    explicit MouseEventSlider(gui::Slider::Type type);
    virtual gui::Widget::EventResult Mouse(const gui::MouseEvent& e) override;
    void SetOnMouseEvent(std::function<void(const gui::MouseEvent&)> handler);

private:
    std::function<void(const gui::MouseEvent& handler)> handler;
};

enum PointInfoEventType {
    REMOVE_CLICKED,
    ICP_CLICKED,
    MERGE_CLICKED,
    READ_MATRIX_CLICKED,
    SHOW_MATRIX_CLICKED,
    SLIDER_MOUSE_RELEASE,
    INDEX_CHANGED,
    NAME_CHANGED,
    SLIDER_VALUE_CHANGED
};

struct PointInfoEvent {
public:
    PointInfoEventType type;
    int entry_index;

    double x_translation;
    double y_translation;
    double z_translation;
    double x_rotation;
    double y_rotation;
    double z_rotation;

    std::string name;

    PointInfoEvent(PointInfo* point_info);
};


class PointInfo : public gui::Vert {
    friend PointInfoEvent;
public:
    PointInfo(int spacing, const gui::Margins& margins = gui::Margins());

    void SetEventHandler(std::function<void(PointInfoEvent&)> handler);
    /// <summary>
    /// Overwrites the displayed text for the currently selected point cloud.
    /// </summary>
    /// <param name="name">Name of the point cloud</param>
    void SetName(const char* name);
    void ResetSliders();
    std::shared_ptr<gui::Combobox> entries;
private:
    std::shared_ptr<gui::TextEdit> name_edit;
    std::shared_ptr<gui::Button> remove;

    std::shared_ptr<MouseEventSlider> x_translation;
    std::shared_ptr<MouseEventSlider> y_translation;
    std::shared_ptr<MouseEventSlider> z_translation;
    std::shared_ptr<MouseEventSlider> x_rotation;
    std::shared_ptr<MouseEventSlider> y_rotation;
    std::shared_ptr<MouseEventSlider> z_rotation;

    std::shared_ptr<gui::Button> icp;
    std::shared_ptr<gui::Button> merge;
    std::shared_ptr<gui::Button> read_matrix;
    std::shared_ptr<gui::Button> show_matrix;

    std::function<void(PointInfoEvent&)> handler;

    std::function<void(void)> make_button_handler(PointInfoEventType type);
};
