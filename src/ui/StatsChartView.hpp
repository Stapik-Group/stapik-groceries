#pragma once

#include <string>
#include <vector>

#include <gtkmm/drawingarea.h>

#include "ui/MoneyFormatter.hpp"

namespace groceries::ui
{
    struct StatsChartPoint
    {
        std::string monthLabel;
        double planned = 0.0;
        double real = 0.0;
        double savings = 0.0;
    };

    class StatsChartView : public Gtk::DrawingArea
    {
    public:
        explicit StatsChartView(const MoneyFormatter &formatter);

        void setData(std::vector<StatsChartPoint> points);

    private:
        const MoneyFormatter &formatter_;
        std::vector<StatsChartPoint> points_;

        void onDraw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
    };
}
