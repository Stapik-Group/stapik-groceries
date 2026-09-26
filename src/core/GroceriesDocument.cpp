#include "core/GroceriesDocument.hpp"

#include <memory>
#include <algorithm>
#include <string>

namespace groceries::core
{
    using domain::Category;
    using domain::MonthId;
    using domain::MonthlyList;

    Category *GroceriesDocument::findCategory(const std::string &id)
    {
        const auto it = std::ranges::find_if(categories_, [&](const Category &category) {
            return category.id() == id;
        });

        return it == categories_.end() ? nullptr : std::to_address(it);
    }

    void GroceriesDocument::addCategory(Category category)
    {
        categories_.push_back(std::move(category));
    }

    bool GroceriesDocument::removeCategory(const std::string &id)
    {
        const auto it = std::ranges::find_if(categories_, [&](const Category &c) { return c.id() == id; });
        if (it == categories_.end())
        {
            return false;
        }
        categories_.erase(it);
        return true;
    }

    MonthlyList *GroceriesDocument::findMonth(const MonthId &id)
    {
        const auto it = std::ranges::find_if(months_, [&](const MonthlyList &month) {
            return month.month() == id;
        });

        return it == months_.end() ? nullptr : std::to_address(it);
    }

    MonthlyList &GroceriesDocument::upsertMonth(MonthId id)
    {
        if (auto *existing = findMonth(id))
        {
            return *existing;
        }
        months_.emplace_back(id);
        std::ranges::sort(months_, [](const MonthlyList &a, const MonthlyList &b) { return a.month() < b.month(); });
        return *findMonth(id);
    }

    bool GroceriesDocument::removeMonth(const MonthId &id)
    {
        const auto it = std::ranges::find_if(months_, [&](const MonthlyList &m) { return m.month() == id; });
        if (it == months_.end())
        {
            return false;
        }
        months_.erase(it);
        return true;
    }

    std::optional<MonthId> GroceriesDocument::latestMonth() const
    {
        if (months_.empty())
        {
            return std::nullopt;
        }
        return std::ranges::max_element(
            months_,
            [](const MonthlyList &a, const MonthlyList &b) { return a.month() < b.month(); }
        )->month();
    }
}
