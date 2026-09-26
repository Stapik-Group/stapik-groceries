#pragma once

#include <functional>

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>

#include "domain/MonthlyList.hpp"
#include "ui/MoneyFormatter.hpp"

namespace groceries::ui
{
    class MonthSummaryPanel : public Gtk::Box
    {
    public:
        explicit MonthSummaryPanel(const MoneyFormatter &formatter);

        void refresh(const domain::MonthlyList &month);

        std::function<void(double newOffBudgetAmount)> onOffBudgetChanged;

    private:
        const MoneyFormatter &formatter_;

        Gtk::Label plannedLabel_;
        Gtk::Label realLabel_;
        Gtk::Label savingsLabel_;
        Gtk::Label offBudgetCaption_;
        Gtk::SpinButton offBudgetSpin_;
        Gtk::Label projectedLabel_;

        bool suppressOffBudgetSignal_ = false;
        const domain::MonthlyList *lastMonth_ = nullptr;
    };
}
