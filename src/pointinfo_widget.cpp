#include <pointinfo_widget.h>

MouseEventSlider::MouseEventSlider(gui::Slider::Type type) : gui::Slider(type) {
    handler = [](const gui::MouseEvent& _e) {};
}

gui::Widget::EventResult MouseEventSlider::Mouse(const gui::MouseEvent& e) {
    gui::Widget::EventResult result = Slider::Mouse(e);
    this->handler(e);
    return result;
}

void MouseEventSlider::SetOnMouseEvent(std::function<void(const gui::MouseEvent&)> handler) {
    this->handler = handler;
}

PointInfoEvent::PointInfoEvent(PointInfo* point_info) {
    type = PointInfoEventType::REMOVE_CLICKED;
    entry_index = point_info->entries->GetSelectedIndex();

    x_translation = point_info->x_translation->GetDoubleValue();
    y_translation = point_info->y_translation->GetDoubleValue();
    z_translation = point_info->z_translation->GetDoubleValue();
    x_rotation = point_info->x_rotation->GetDoubleValue();
    y_rotation = point_info->y_rotation->GetDoubleValue();
    z_rotation = point_info->z_rotation->GetDoubleValue();

    name = std::string(point_info->name_edit->GetText());
}


PointInfo::PointInfo(int spacing, const gui::Margins& margins) : gui::Vert(spacing, margins) {
    auto& app = gui::Application::GetInstance();
    auto& theme = app.GetTheme();
    const float em = (float) theme.font_size;
    const int lm = int(std::ceil(0.5 * em));
    const int grid_spacing = int(std::ceil(0.25 * em));
    const int separation_height = int(std::ceil(0.75 * em));
    gui::Margins indent(em, 0, 0, 0);

    handler = [](PointInfoEvent& _) {};

    entries = std::make_shared<gui::Combobox>();
    remove = std::make_shared<gui::Button>("L\xC3\xB6schen"); // "Löschen"

    // Entry name

    name_edit = std::make_shared<gui::TextEdit>();
    auto name_edit_desc = std::make_shared<gui::Label>("Name");
    auto name = std::make_shared<gui::Horiz>(grid_spacing);
    name->AddChild(name_edit_desc);
    name->AddChild(name_edit);

    // Remove Button


    // Sliders

    x_translation = std::make_shared<MouseEventSlider>(gui::Slider::Type::DOUBLE);
    y_translation = std::make_shared<MouseEventSlider>(gui::Slider::Type::DOUBLE);
    z_translation = std::make_shared<MouseEventSlider>(gui::Slider::Type::DOUBLE);
    x_rotation = std::make_shared<MouseEventSlider>(gui::Slider::Type::DOUBLE);
    y_rotation = std::make_shared<MouseEventSlider>(gui::Slider::Type::DOUBLE);
    z_rotation = std::make_shared<MouseEventSlider>(gui::Slider::Type::DOUBLE);

    x_translation->SetValue(0.0);
    x_translation->SetLimits(-10000.0, 10000.0);
    y_translation->SetValue(0.0);
    y_translation->SetLimits(-10000.0, 10000.0);
    z_translation->SetValue(0.0);
    z_translation->SetLimits(-10000.0, 10000.0);

    const double PI = 3.1415926535898;

    x_rotation->SetValue(0.0);
    x_rotation->SetLimits(-PI, PI);
    y_rotation->SetValue(0.0);
    y_rotation->SetLimits(-PI, PI);
    z_rotation->SetValue(0.0);
    z_rotation->SetLimits(-PI, PI);

    gui::Color red = gui::Color(1.0, 0.0, 0.0);
    gui::Color green = gui::Color(0.0, 1.0, 0.0);
    gui::Color blue = gui::Color(0.0, 0.0, 1.0);

    auto x_axis_label_1 = std::make_shared<gui::Label>("X-Achse");
    auto y_axis_label_1 = std::make_shared<gui::Label>("Y-Achse");
    auto z_axis_label_1 = std::make_shared<gui::Label>("Z-Achse");
    x_axis_label_1->SetTextColor(red);
    y_axis_label_1->SetTextColor(green);
    z_axis_label_1->SetTextColor(blue);

    auto translation_vert = std::make_shared<gui::CollapsableVert>("Translation", 0, indent);
    auto translation_grid = std::make_shared<gui::VGrid>(2, grid_spacing);
    translation_vert->SetIsOpen(true);
    translation_vert->AddChild(translation_grid);

    translation_grid->AddChild(x_axis_label_1);
    translation_grid->AddChild(x_translation);
    translation_grid->AddChild(y_axis_label_1);
    translation_grid->AddChild(y_translation);
    translation_grid->AddChild(z_axis_label_1);
    translation_grid->AddChild(z_translation);

    auto x_axis_label_2 = std::make_shared<gui::Label>("X-Achse");
    auto y_axis_label_2 = std::make_shared<gui::Label>("Y-Achse");
    auto z_axis_label_2 = std::make_shared<gui::Label>("Z-Achse");
    x_axis_label_2->SetTextColor(red);
    y_axis_label_2->SetTextColor(green);
    z_axis_label_2->SetTextColor(blue);

    auto rotation_vert = std::make_shared<gui::CollapsableVert>("Rotation", 0, indent);
    auto rotation_grid = std::make_shared<gui::VGrid>(2, grid_spacing);
    rotation_vert->SetIsOpen(true);
    rotation_vert->AddChild(rotation_grid);

    rotation_grid->AddChild(x_axis_label_2);
    rotation_grid->AddChild(x_rotation);
    rotation_grid->AddChild(y_axis_label_2);
    rotation_grid->AddChild(y_rotation);
    rotation_grid->AddChild(z_axis_label_2);
    rotation_grid->AddChild(z_rotation);

    // Other Buttons

    auto misc_buttons = std::make_shared<gui::Horiz>(grid_spacing);
    icp = std::make_shared<gui::Button>("Algorithmisches Ann\xC3\xA4hern"); // "Algorithmisches Ann�hern"
    merge = std::make_shared<gui::Button>("Verschmelzen");

    misc_buttons->AddChild(icp);
    misc_buttons->AddChild(merge);

    // Matrix Buttons

    auto matrix_buttons = std::make_shared<gui::Horiz>(grid_spacing);
    read_matrix = std::make_shared<gui::Button>("Matrix Einlesen");
    show_matrix = std::make_shared<gui::Button>("Matrix Ausgeben");

    matrix_buttons->AddChild(read_matrix);
    matrix_buttons->AddChild(show_matrix);

    // Construct Widget

    AddFixed(separation_height);
    AddChild(entries);
    AddFixed(separation_height);
    AddChild(name);
    AddFixed(separation_height);
    AddChild(remove);
    AddFixed(separation_height);
    AddChild(translation_vert);
    AddChild(rotation_vert);
    AddFixed(separation_height);
    AddChild(misc_buttons);
    AddFixed(separation_height);
    AddChild(matrix_buttons);

    // Handle Events

    remove->SetOnClicked(make_button_handler(PointInfoEventType::REMOVE_CLICKED));

    name_edit->SetOnValueChanged([this](const char* text) {
        if (this->entries->GetNumberOfItems() == 0) return;
        PointInfoEvent event_(this);
        event_.name = std::string(text);
        event_.type = PointInfoEventType::NAME_CHANGED;
        this->handler(event_);
        });

    icp->SetOnClicked(make_button_handler(PointInfoEventType::ICP_CLICKED));
    merge->SetOnClicked(make_button_handler(PointInfoEventType::MERGE_CLICKED));
    read_matrix->SetOnClicked(make_button_handler(PointInfoEventType::READ_MATRIX_CLICKED));
    show_matrix->SetOnClicked(make_button_handler(PointInfoEventType::SHOW_MATRIX_CLICKED));

    entries->SetOnValueChanged([this](const char* str, int i) {
        if (this->entries->GetNumberOfItems() == 0) return;
        PointInfoEvent event_(this);
        event_.entry_index = i;
        event_.type = PointInfoEventType::INDEX_CHANGED;
        this->handler(event_);
        });

    auto value_change_handler = ([this](double _d) {
        if (this->entries->GetNumberOfItems() == 0) return;
        PointInfoEvent event_(this);
        event_.type = PointInfoEventType::SLIDER_VALUE_CHANGED;
        this->handler(event_);
        });

    x_translation->SetOnValueChanged(value_change_handler);
    y_translation->SetOnValueChanged(value_change_handler);
    z_translation->SetOnValueChanged(value_change_handler);
    x_rotation->SetOnValueChanged(value_change_handler);
    y_rotation->SetOnValueChanged(value_change_handler);
    z_rotation->SetOnValueChanged(value_change_handler);

    auto slider_mouse_handler = ([this](const gui::MouseEvent& e) {
        if (e.type != gui::MouseEvent::Type::BUTTON_UP) return;
        PointInfoEvent event_(this);
        event_.type = PointInfoEventType::SLIDER_MOUSE_RELEASE;
        this->handler(event_);
        });

    x_translation->SetOnMouseEvent(slider_mouse_handler);
    y_translation->SetOnMouseEvent(slider_mouse_handler);
    z_translation->SetOnMouseEvent(slider_mouse_handler);
    x_rotation->SetOnMouseEvent(slider_mouse_handler);
    y_rotation->SetOnMouseEvent(slider_mouse_handler);
    z_rotation->SetOnMouseEvent(slider_mouse_handler);
}


void PointInfo::SetName(const char* name) {
    this->entries->ChangeItem(this->entries->GetSelectedValue(), name);
    this->name_edit->SetText(name);
}

void PointInfo::SetEventHandler(std::function<void(PointInfoEvent&)> handler) {
    this->handler = handler;
}

void PointInfo::ResetSliders() {
    x_translation->SetValue(0.0);
    y_translation->SetValue(0.0);
    z_translation->SetValue(0.0);
    x_rotation->SetValue(0.0);
    y_rotation->SetValue(0.0);
    z_rotation->SetValue(0.0);
}

std::function<void(void)> PointInfo::make_button_handler(PointInfoEventType type) {
    auto button_handler = ([this, type]() {
        if (this->entries->GetNumberOfItems() == 0) return;
        PointInfoEvent event_(this);
        event_.type = type;
        this->handler(event_);
        });

    return button_handler;
}
