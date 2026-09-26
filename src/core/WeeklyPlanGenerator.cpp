#include "core/WeeklyPlanGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <chrono>

namespace groceries::core
{
    using domain::Category;
    using domain::MonthlyList;
    using domain::Product;

    namespace
    {
        bool isLeapYear(const int year)
        {
            return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
        }

        int daysInMonth(const int year, const int month)
        {
            static constexpr int kDaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            if (month == 2 && isLeapYear(year))
            {
                return 29;
            }
            return kDaysInMonth[month - 1];
        }

        int isoWeekday(const int year, const int month, const int day)
        {
            const std::chrono::year_month_day ymd{
                std::chrono::year{year},
                std::chrono::month{static_cast<unsigned>(month)},
                std::chrono::day{static_cast<unsigned>(day)}
            };

            const std::chrono::weekday wd{std::chrono::sys_days{ymd}};
            return static_cast<int>(wd.iso_encoding());
        }

        int weekNumberInMonth(const int year, const int month, const int day)
        {
            const int firstWeekday = isoWeekday(year, month, 1);
            return (day + firstWeekday - 2) / 7 + 1;
        }

        const Category *findCategory(const std::vector<Category> &categories, const std::string &categoryId)
        {
            for (const Category &category: categories)
            {
                if (category.id() == categoryId)
                {
                    return &category;
                }
            }
            return nullptr;
        }
    }

    WeeklyPlanGenerator::Result WeeklyPlanGenerator::generate(const MonthlyList &month, const std::vector<Category> &categories, const std::tm &referenceDate)
    {
        const int year = referenceDate.tm_year + 1900;
        const int monthNum = referenceDate.tm_mon + 1;
        const int day = referenceDate.tm_mday;

        const int totalDaysInMonth = daysInMonth(year, monthNum);
        const int currentWeek = weekNumberInMonth(year, monthNum, day);
        const int totalWeeks = weekNumberInMonth(year, monthNum, totalDaysInMonth);
        const int weeksRemaining = std::max(1, totalWeeks - currentWeek + 1);

        Result result;
        result.currentWeekNumber = currentWeek;
        result.totalWeeksInMonth = totalWeeks;

        for (const Product &product: month.products())
        {
            constexpr double kEpsilon = 1e-6;
            const double remaining = product.plannedQuantity() - product.purchasedQuantity();
            if (remaining <= kEpsilon)
            {
                continue;
            }

            const double roundedRemaining = std::round(remaining);
            const bool isWholeUnits = std::abs(remaining - roundedRemaining) < kEpsilon;

            double thisWeekQuantity;
            if (isWholeUnits)
            {
                const auto wholeRemaining = static_cast<long long>(roundedRemaining);
                const auto weeks = static_cast<long long>(weeksRemaining);
                const auto roundedQuantity = (wholeRemaining + weeks - 1) / weeks;
                thisWeekQuantity = static_cast<double>(roundedQuantity);
            }
            else
            {
                thisWeekQuantity = remaining / static_cast<double>(weeksRemaining);
            }

            if (thisWeekQuantity <= kEpsilon)
            {
                continue;
            }

            const Category *category = findCategory(categories, product.categoryId());
            result.items.emplace_back(
                product.name(),
                category != nullptr ? category->name() : std::string(),
                thisWeekQuantity,
                product.unitName(),
                isWholeUnits,
                product.plannedUnitPrice()
            );
        }

        return result;
    }
}