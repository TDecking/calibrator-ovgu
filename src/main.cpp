
#include <string>

#include "open3d/Open3D.h"
#include "main_window.h"

int main(int argc, const char** argv) {
    if (argc == 2) {
        std::string option(argv[1]);
        if (option == "--skip-for-unit-test") {
            open3d::utility::LogInfo("Skiped for unit test.");
            return 0;
        }
    }

    auto& app = open3d::visualization::gui::Application::GetInstance();
    app.Initialize(argc, argv);

    std::shared_ptr<MainWindow> gui = std::make_shared<MainWindow>("Calibrator", 1024, 768);

    app.AddWindow(gui);
    gui.reset();
    app.Run();

    return 0;
}
