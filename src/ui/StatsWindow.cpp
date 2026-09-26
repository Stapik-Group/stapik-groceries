#include "ui/StatsWindow.hpp"

#include <cstdio>

#include <gtkmm/box.h>
#include <gtkmm/label.h>

#include <algorithm>
#include <ranges>

#include "stapik/locale/LocaleManager.hpp"

namespace groceries::ui
{
    using domain::MonthlyList;

    namespace
    {
        const char *const kMonthKeys[] = {
            "month.january", "month.february", "month.march", "month.april",
            "month.may", "month.june", "month.july", "month.august",
            "month.september", "month.october", "month.november", "month.december"
        };

        std::string formatMonthLabel(const domain::MonthId &id)
        {
            const std::string name = (id.month >= 1 && id.month <= 12)
                                         ? LocaleManager::instance().translate(kMonthKeys[id.month - 1])
                                         : "?";
            return name + " " + std::to_string(id.year);
        }
    }

    StatsWindow::StatsWindow(Gtk::Window &parent,
                             const std::vector<MonthlyList> &months,
                             const MoneyFormatter &formatter)
        : chartView_(formatter)
    {
        const auto &loc = LocaleManager::instance();

        set_transient_for(parent);
        set_default_size(640, 460);
        set_title(loc.translate("stats.title"));

        listViewButton_.set_label(loc.translate("stats.view.list"));
        chartViewButton_.set_label(loc.translate("stats.view.chart"));
        chartViewButton_.set_group(listViewButton_);
        listViewButton_.set_active(true);
        viewSwitchBar_.set_margin(8);
        viewSwitchBar_.append(listViewButton_);
        viewSwitchBar_.append(chartViewButton_);
        rootBox_.append(viewSwitchBar_);

        scroller_.set_child(listBox_);
        scroller_.set_vexpand(true);
        listBox_.set_selection_mode(Gtk::SelectionMode::NONE);

        viewStack_.set_vexpand(true);
        viewStack_.add(scroller_, "list");
        viewStack_.add(chartView_, "chart");
        rootBox_.append(viewStack_);
        rootBox_.add_css_class("groceries-window-frame");
        set_child(rootBox_);

        listViewButton_.signal_toggled().connect([this]()
        {
            if (listViewButton_.get_active())
            {
                viewStack_.set_visible_child("list");
            }
        });
        chartViewButton_.signal_toggled().connect([this]()
        {
            if (chartViewButton_.get_active())
            {
                viewStack_.set_visible_child("chart");
            }
        });

        for (const auto & month : std::views::reverse(months))
        {
            auto row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 2);
            row->set_margin(8);

            auto titleLabel = Gtk::make_managed<Gtk::Label>(formatMonthLabel(month.month()));
            titleLabel->set_xalign(0.0f);
            titleLabel->add_css_class("groceries-stats-heading");
            row->append(*titleLabel);

            auto detailsLabel = Gtk::make_managed<Gtk::Label>(
                loc.translate("summary.planned") + ": " + formatter.format(month.plannedTotal()) +
                "   " + loc.translate("summary.real") + ": " + formatter.format(month.realTotal()) +
                "   " + loc.translate("summary.savings") + ": " + formatter.format(month.savingsTotal()) +
                "   " + loc.translate("summary.offBudget") + ": " + formatter.format(month.offBudgetAmount()) +
                "   " + loc.translate("stats.projected") + ": " + formatter.format(month.projectedMonthlyCost()));
            detailsLabel->set_xalign(0.0f);
            row->append(*detailsLabel);

            listBox_.append(*row);
        }

        std::vector<StatsChartPoint> chartPoints;
        const std::size_t chartStart = months.size() > 12 ? months.size() - 12 : 0;
        for (std::size_t i = chartStart; i < months.size(); ++i)
        {
            const MonthlyList &month = months[i];
            chartPoints.push_back({
                formatMonthLabel(month.month()), month.plannedTotal(),
                month.realTotal(), month.savingsTotal()
            });
        }
        chartView_.setData(std::move(chartPoints));

        present();
        signal_hide().connect([this]() { delete this; });
    }
}
