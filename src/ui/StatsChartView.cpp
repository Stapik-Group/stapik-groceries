#include "ui/StatsChartView.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

#include <pangomm/layout.h>

#include "stapik/locale/LocaleManager.hpp"

namespace groceries::ui
{
    namespace
    {
        constexpr double kPlannedColor[3] = {0.30, 0.45, 0.75};
        constexpr double kRealColor[3] = {0.80, 0.35, 0.20};
        constexpr double kSavingsColor[3] = {0.30, 0.65, 0.35};

        constexpr int kLeftMargin = 56;
        constexpr int kBottomMargin = 42;
        constexpr int kTopMargin = 12;
        constexpr int kRightMargin = 12;
        constexpr int kLegendHeight = 22;

        void drawText(const Cairo::RefPtr<Cairo::Context> &cr, const std::string &text, double x, double y,
                      double r, double g, double b, bool centered = false)
        {
            auto layout = Pango::Layout::create(cr);
            layout->set_text(text);
            if (centered)
            {
                int textWidth = 0;
                int textHeight = 0;
                layout->get_pixel_size(textWidth, textHeight);
                x -= textWidth / 2.0;
            }
            cr->save();
            cr->set_source_rgb(r, g, b);
            cr->move_to(x, y);
            layout->show_in_cairo_context(cr);
            cr->restore();
        }
    }

    StatsChartView::StatsChartView(const MoneyFormatter &formatter) : formatter_(formatter)
    {
        set_content_width(560);
        set_content_height(320);
        set_hexpand(true);
        set_vexpand(true);
        set_draw_func(sigc::mem_fun(*this, &StatsChartView::onDraw));
    }

    void StatsChartView::setData(std::vector<StatsChartPoint> points)
    {
        points_ = std::move(points);
        queue_draw();
    }

    void StatsChartView::onDraw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height)
    {
        if (points_.empty())
        {
            return;
        }

        double maxValue = 0.0;
        for (const StatsChartPoint &point: points_)
        {
            maxValue = std::max({maxValue, point.planned, point.real, point.savings});
        }
        if (maxValue <= 0.0)
        {
            maxValue = 1.0;
        }

        const double plotLeft = kLeftMargin;
        const double plotRight = width - kRightMargin;
        const double plotTop = kTopMargin;
        const double plotBottom = height - kBottomMargin - kLegendHeight;
        const double plotHeight = std::max(1.0, plotBottom - plotTop);
        const double plotWidth = std::max(1.0, plotRight - plotLeft);

        cr->set_line_width(1.0);
        for (int i = 0; i <= 2; ++i)
        {
            const double fraction = i / 2.0;
            const double y = plotBottom - plotHeight * fraction;
            cr->set_source_rgba(0.5, 0.5, 0.5, 0.35);
            cr->move_to(plotLeft, y);
            cr->line_to(plotRight, y);
            cr->stroke();
            drawText(cr, formatter_.format(maxValue * fraction), plotLeft - 8, y - 7, 0.4, 0.4, 0.4);
        }

        const double groupWidth = plotWidth / static_cast<double>(points_.size());
        const double barGap = 4.0;
        const double barWidth = std::max(2.0, (groupWidth - barGap * 4) / 3.0);

        for (std::size_t i = 0; i < points_.size(); ++i)
        {
            const StatsChartPoint &point = points_[i];
            const double groupLeft = plotLeft + groupWidth * static_cast<double>(i);
            double barX = groupLeft + barGap;

            const double values[3] = {point.planned, point.real, point.savings};
            const double *colors[3] = {kPlannedColor, kRealColor, kSavingsColor};
            for (int s = 0; s < 3; ++s)
            {
                const double barHeight = plotHeight * (values[s] / maxValue);
                cr->set_source_rgb(colors[s][0], colors[s][1], colors[s][2]);
                cr->rectangle(barX, plotBottom - barHeight, barWidth, barHeight);
                cr->fill();
                barX += barWidth + barGap;
            }

            drawText(cr, point.monthLabel, groupLeft + groupWidth / 2.0, plotBottom + 6, 0.2, 0.2, 0.2, true);
        }

        double legendX = plotLeft;
        const double legendY = height - kLegendHeight + 4;
        const auto &loc = LocaleManager::instance();
        const struct
        {
            std::string label;
            const double *color;
        } legendItems[3] = {
            {loc.translate("summary.planned"), kPlannedColor},
            {loc.translate("summary.real"), kRealColor},
            {loc.translate("summary.savings"), kSavingsColor}
        };
        for (const auto &item: legendItems)
        {
            cr->set_source_rgb(item.color[0], item.color[1], item.color[2]);
            cr->rectangle(legendX, legendY, 10, 10);
            cr->fill();
            drawText(cr, item.label, legendX + 14, legendY - 3, 0.2, 0.2, 0.2);
            legendX += 14 + 8.0 * static_cast<double>(item.label.size()) + 20;
        }
    }
}
