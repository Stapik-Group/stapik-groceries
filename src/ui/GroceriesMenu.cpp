#include "ui/GroceriesMenu.hpp"

#include <glibmm/ustring.h>
#include <glibmm/variant.h>

#include "domain/Currency.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/theme/ThemeManager.hpp"

namespace groceries::ui
{
    GroceriesMenu::GroceriesMenu(Gtk::ApplicationWindow &window,
                                 GroceriesMenuCallbacks callbacks)
        : window_(window),
          callbacks_(std::move(callbacks))
    {
        initFileActions();
        initMonthAction();
        initLanguageAction();
        initThemeAction();
        initCurrencyAction();
        buildModel();

        LocaleManager::instance().signalLocaleChanged().connect([this]
        {
            buildModel();
        });
    }

    Gtk::PopoverMenuBar &GroceriesMenu::getMenuBar()
    {
        return menuBar_;
    }

    void GroceriesMenu::setMonths(const std::vector<Glib::ustring> &labels, const std::size_t selectedIndex)
    {
        monthLabels_ = labels;
        selectMonthAction_->change_state(static_cast<int>(selectedIndex));
        buildModel();
    }

    void GroceriesMenu::buildModel()
    {
        const auto &loc = LocaleManager::instance();

        menuModel_ = Gio::Menu::create();

        const auto menuFile = Gio::Menu::create();
        menuFile->append(loc.translate("header.newMonth"), "win.newMonth");
        menuFile->append(loc.translate("header.categories"), "win.categories");
        menuFile->append(loc.translate("header.stats"), "win.stats");

        const auto menuFileWeeklyPlan = Gio::Menu::create();
        menuFileWeeklyPlan->append(
            loc.translate("menu.file.weeklyPlan"),
            "win.weeklyPlan");
        menuFile->append_section(menuFileWeeklyPlan);

        const auto menuFileCloud = Gio::Menu::create();
        menuFileCloud->append(loc.translate("menu.file.connect"), "win.connect");
        menuFileCloud->append(loc.translate("menu.file.sync"), "win.sync");
        menuFile->append_section(menuFileCloud);

        const auto menuFileQuit = Gio::Menu::create();
        menuFileQuit->append(loc.translate("menu.file.quit"), "win.quit");
        menuFile->append_section(menuFileQuit);
        menuModel_->append_submenu(loc.translate("menu.file"), menuFile);

        const auto menuMonth = Gio::Menu::create();

        for (std::size_t i = 0; i < monthLabels_.size(); ++i)
        {
            menuMonth->append(
                monthLabels_[i],
                Glib::ustring::compose("win.selectMonth(%1)", i));
        }

        const auto menuMonthManage = Gio::Menu::create();
        menuMonthManage->append(
            loc.translate("menu.month.delete"),
            "win.deleteMonth");
        menuMonth->append_section(menuMonthManage);
        menuModel_->append_submenu(loc.translate("menu.month"), menuMonth);

        const auto menuLanguage = Gio::Menu::create();
        menuLanguage->append(
            loc.translate("menu.settings.language.pl"),
            "win.setLanguage::pl");
        menuLanguage->append(
            loc.translate("menu.settings.language.en"),
            "win.setLanguage::en");
        menuLanguage->append(
            loc.translate("menu.settings.language.de"),
            "win.setLanguage::de");

        const auto menuTheme = Gio::Menu::create();
        menuTheme->append(
            loc.translate("menu.settings.theme.classic"),
            "win.setTheme::classic");
        menuTheme->append(
            loc.translate("menu.settings.theme.classicPink"),
            "win.setTheme::classic-pink");
        menuTheme->append(
            loc.translate("menu.settings.theme.modern"),
            "win.setTheme::modern");

        const auto menuSettings = Gio::Menu::create();
        menuSettings->append_submenu(
            loc.translate("menu.settings.language"),
            menuLanguage);
        menuSettings->append_submenu(
            loc.translate("menu.settings.theme"),
            menuTheme);

        const auto menuCurrency = Gio::Menu::create();

        for (const domain::Currency &currency : domain::CurrencyCatalog::all())
        {
            menuCurrency->append(
                currency.code + " (" + currency.symbol + ")",
                "win.setCurrency::" + currency.code);
        }

        menuSettings->append_submenu(
            loc.translate("menu.settings.currency"),
            menuCurrency);
        menuModel_->append_submenu(
            loc.translate("menu.settings"),
            menuSettings);

        menuBar_.set_menu_model(menuModel_);
    }

    void GroceriesMenu::initFileActions() const
    {
        window_.add_action("quit", [this]
        {
            window_.get_application()->quit();
        });

        window_.add_action("newMonth", [this]
        {
            callbacks_.onNewMonth();
        });

        window_.add_action("categories", [this]
        {
            callbacks_.onCategories();
        });

        window_.add_action("stats", [this]
        {
            callbacks_.onStats();
        });

        window_.add_action("deleteMonth", [this]
        {
            callbacks_.onDeleteMonth();
        });

        window_.add_action("connect", [this]
        {
            callbacks_.onConnect();
        });

        window_.add_action("sync", [this]
        {
            callbacks_.onSync();
        });

        window_.add_action("weeklyPlan", [this]
        {
            callbacks_.onWeeklyPlan();
        });
    }

    void GroceriesMenu::initMonthAction()
    {
        selectMonthAction_ = Gio::SimpleAction::create_radio_integer("selectMonth", 0);

        selectMonthAction_->signal_activate().connect(
            [this](const Glib::VariantBase &parameter)
            {
                const auto index =
                    Glib::VariantBase::cast_dynamic<Glib::Variant<int>>(parameter).get();

                if (index >= 0 && static_cast<std::size_t>(index) < monthLabels_.size())
                {
                    selectMonthAction_->change_state(index);
                    callbacks_.onSelectMonth(static_cast<std::size_t>(index));
                }
            });

        window_.add_action(selectMonthAction_);
    }

    void GroceriesMenu::initLanguageAction() const
    {
        using enum Locale;

        const auto initialLocale = LocaleManager::instance().getLocale();
        std::string initialValue = "pl";

        if (initialLocale == EN)
        {
            initialValue = "en";
        }
        else if (initialLocale == DE)
        {
            initialValue = "de";
        }

        auto action =
            Gio::SimpleAction::create_radio_string("setLanguage", initialValue);

        action->signal_activate().connect(
            [action](const Glib::VariantBase &parameter)
            {
                const auto value =
                    Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(
                        parameter).get();

                action->change_state(value);

                if (value == "pl")
                {
                    LocaleManager::instance().setLocale(PL);
                }
                else if (value == "en")
                {
                    LocaleManager::instance().setLocale(EN);
                }
                else if (value == "de")
                {
                    LocaleManager::instance().setLocale(DE);
                }
            });

        window_.add_action(action);
    }

    void GroceriesMenu::setCurrency(const std::string &code) const
    {
        setCurrencyAction_->change_state(Glib::ustring(code));
    }

    void GroceriesMenu::initCurrencyAction()
    {
        setCurrencyAction_ = Gio::SimpleAction::create_radio_string(
            "setCurrency",
            domain::CurrencyCatalog::defaultCurrency().code);

        setCurrencyAction_->signal_activate().connect(
            [this](const Glib::VariantBase &parameter)
            {
                const auto value =
                    Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(
                        parameter).get();

                setCurrencyAction_->change_state(value);
                callbacks_.onSetCurrency(value.raw());
            });

        window_.add_action(setCurrencyAction_);
    }

    void GroceriesMenu::initThemeAction() const
    {
        const auto currentTheme = ThemeManager::instance().getTheme();
        std::string initialValue = "classic";

        if (currentTheme == Theme::Modern)
        {
            initialValue = "modern";
        }
        else if (currentTheme == Theme::ClassicPink)
        {
            initialValue = "classic-pink";
        }

        auto action =
            Gio::SimpleAction::create_radio_string("setTheme", initialValue);

        action->signal_activate().connect(
            [action](const Glib::VariantBase &parameter)
            {
                using enum Theme;

                const auto value =
                    Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(
                        parameter).get();

                action->change_state(value);

                if (value == "modern")
                {
                    ThemeManager::instance().setTheme(Modern);
                }
                else if (value == "classic-pink")
                {
                    ThemeManager::instance().setTheme(ClassicPink);
                }
                else
                {
                    ThemeManager::instance().setTheme(Classic);
                }
            });

        window_.add_action(action);
    }
}