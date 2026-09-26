#include "ui/MonthSummaryPanel.hpp"

#include "stapik/locale/LocaleManager.hpp"

namespace groceries::ui
{
    using domain::MonthlyList;

    MonthSummaryPanel::MonthSummaryPanel(const MoneyFormatter &formatter)
        : Gtk::Box(Gtk::Orientation::HORIZONTAL, 16), formatter_(formatter)
    {
        set_margin(8);
        add_css_class("groceries-summary");

        for (Gtk::Label *label: {&plannedLabel_, &realLabel_, &savingsLabel_})
        {
            label->set_halign(Gtk::Align::START);
            label->add_css_class("summary-figure");
            append(*label);
        }

        offBudgetCaption_.set_halign(Gtk::Align::START);
        offBudgetCaption_.add_css_class("summary-caption");
        append(offBudgetCaption_);

        offBudgetSpin_.set_range(0.0, 100000.0);
        offBudgetSpin_.set_increments(1.0, 10.0);
        offBudgetSpin_.set_digits(2);
        append(offBudgetSpin_);

        projectedLabel_.set_halign(Gtk::Align::START);
        projectedLabel_.add_css_class("summary-figure");
        append(projectedLabel_);

        offBudgetSpin_.signal_value_changed().connect([this]()
        {
            if (suppressOffBudgetSignal_)
            {
                return;
            }
            if (onOffBudgetChanged)
            {
                onOffBudgetChanged(offBudgetSpin_.get_value());
            }
        });

        LocaleManager::instance().signalLocaleChanged().connect([this]
        {
            if (lastMonth_ != nullptr)
            {
                refresh(*lastMonth_);
            }
        });
    }

    void MonthSummaryPanel::refresh(const MonthlyList &month)
    {
        lastMonth_ = &month;
        const auto &loc = LocaleManager::instance();

        plannedLabel_.set_text(loc.translate("summary.planned") + ": " + formatter_.format(month.plannedTotal()));
        realLabel_.set_text(loc.translate("summary.real") + ": " + formatter_.format(month.realTotal()));
        savingsLabel_.set_text(loc.translate("summary.savings") + ": " + formatter_.format(month.savingsTotal()));
        offBudgetCaption_.set_text(loc.translate("summary.offBudget") + ":");

        suppressOffBudgetSignal_ = true;
        offBudgetSpin_.set_value(month.offBudgetAmount());
        suppressOffBudgetSignal_ = false;

        projectedLabel_.set_text(loc.translate("summary.projected") + ": " +
                                 formatter_.format(month.projectedMonthlyCost()));
    }
}
