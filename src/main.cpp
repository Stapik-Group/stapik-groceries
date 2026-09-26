#include <gtkmm/application.h>

#include "AppInfo.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/theme/ThemeManager.hpp"
#include "stapik/ui/style/AppStyleProvider.hpp"
#include "ui/MainWindow.hpp"

int main(const int argc, char *argv[])
{
    const auto app = Gtk::Application::create("pl.stapik.groceries");

    AppStyleProvider styleProvider(AppPaths::resourcesDir());

    app->signal_activate().connect([&]
    {
        LocaleManager::instance(APP_NAME);

        styleProvider.apply(ThemeManager::instance(APP_NAME).getTheme());
        ThemeManager::instance().signalThemeChanged().connect(
            [&styleProvider] { styleProvider.apply(ThemeManager::instance().getTheme()); });

        auto *window = new groceries::ui::MainWindow();
        app->add_window(*window);
        window->show();
    });
    return app->run(argc, argv);
}
