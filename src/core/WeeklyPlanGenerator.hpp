#pragma once

#include <ctime>
#include <string>
#include <vector>

#include "domain/Category.hpp"
#include "domain/MonthlyList.hpp"

namespace groceries::core
{
    struct WeeklyPlanItem
    {
        std::string productName;
        std::string categoryName;
        double quantity = 0.0;
        std::string unitName;
        bool isWholeUnits = false;
        double plannedUnitPrice = 0.0;
    };

    class WeeklyPlanGenerator
    {
    public:
        struct Result
        {
            std::vector<WeeklyPlanItem> items;
            int currentWeekNumber = 1;
            int totalWeeksInMonth = 1;
        };

        [[nodiscard]] static Result generate(const domain::MonthlyList &month,
                                             const std::vector<domain::Category> &categories,
                                             const std::tm &referenceDate);
    };
}
