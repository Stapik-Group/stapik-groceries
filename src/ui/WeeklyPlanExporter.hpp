#pragma once

#include <filesystem>

#include "core/WeeklyPlanGenerator.hpp"
#include "ui/MoneyFormatter.hpp"

namespace groceries::ui
{
    class WeeklyPlanExporter
    {
    public:
        static std::filesystem::path exportToHtml(const core::WeeklyPlanGenerator::Result &plan,
                                                  const MoneyFormatter &formatter,
                                                  const std::filesystem::path &outputPath);
    };
}
