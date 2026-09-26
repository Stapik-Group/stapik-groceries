#pragma once

#include <functional>
#include <string>
#include <vector>

#include <gtkmm/applicationwindow.h>
#include <gtkmm/popovermenubar.h>
#include <giomm/menu.h>
#include <giomm/simpleaction.h>

namespace groceries::ui
{
    struct GroceriesMenuCallbacks
    {
        std::function<void()> onNewMonth;
        std::function<void()> onCategories;
        std::function<void()> onStats;
        std::function<void(std::size_t)> onSelectMonth;
        std::function<void()> onDeleteMonth;
        std::function<void(const std::string &)> onSetCurrency;
        std::function<void()> onConnect;
        std::function<void()> onSync;
        std::function<void()> onWeeklyPlan;
    };

    class GroceriesMenu
    {
    public:
        GroceriesMenu(Gtk::ApplicationWindow &window, GroceriesMenuCallbacks callbacks);
        ~GroceriesMenu() = default;
        Gtk::PopoverMenuBar &getMenuBar();

        void setMonths(const std::vector<Glib::ustring> &labels, std::size_t selectedIndex);
        void setCurrency(const std::string &code) const;
    private:
        Gtk::ApplicationWindow &window_;
        Glib::RefPtr<Gio::Menu> menuModel_;
        Gtk::PopoverMenuBar menuBar_;

        GroceriesMenuCallbacks callbacks_;

        std::vector<Glib::ustring> monthLabels_;
        Glib::RefPtr<Gio::SimpleAction> selectMonthAction_;
        Glib::RefPtr<Gio::SimpleAction> setCurrencyAction_;

        void buildModel();
        void initFileActions() const;
        void initMonthAction();
        void initLanguageAction() const;
        void initCurrencyAction();
        void initThemeAction() const;
    };
}