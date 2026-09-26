#pragma once

#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stack.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/window.h>

#include "domain/MonthlyList.hpp"
#include "ui/MoneyFormatter.hpp"
#include "ui/StatsChartView.hpp"

namespace groceries::ui
{
    class StatsWindow : public Gtk::Window
    {
    public:
        StatsWindow(Window &parent,
                    const std::vector<domain::MonthlyList> &months,
                    const MoneyFormatter &formatter);

    private:
        Gtk::Box rootBox_{Gtk::Orientation::VERTICAL};
        Gtk::Box viewSwitchBar_{Gtk::Orientation::HORIZONTAL, 8};
        Gtk::ToggleButton listViewButton_;
        Gtk::ToggleButton chartViewButton_;
        Gtk::Stack viewStack_;

        Gtk::ScrolledWindow scroller_;
        Gtk::ListBox listBox_;
        StatsChartView chartView_;
    };
}
