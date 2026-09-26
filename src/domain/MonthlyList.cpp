#include "domain/MonthlyList.hpp"

#include <algorithm>
#include <format>
#include <memory>
#include <numeric>
#include <stdexcept>

namespace groceries::domain
{
    std::string MonthId::toKey() const
    {
        return std::format("{:04d}-{:02d}", year, month);
    }

    MonthId MonthId::fromKey(const std::string &key)
    {
        if (key.size() != 7 || key[4] != '-')
        {
            throw std::invalid_argument("Invalid month key: " + key);
        }

        MonthId id;
        id.year = std::stoi(key.substr(0, 4));
        id.month = std::stoi(key.substr(5, 2));
        return id;
    }

    MonthlyList::MonthlyList(const MonthId month) : month_(month)
    {
    }

    Product *MonthlyList::findProduct(const std::string &productId)
    {
        const auto it = std::ranges::find_if(products_, [&](const Product &p)
        {
            return p.id() == productId;
        });

        return it == products_.end() ? nullptr : std::to_address(it);
    }

    void MonthlyList::addProduct(Product product)
    {
        products_.push_back(std::move(product));
    }

    bool MonthlyList::removeProduct(const std::string &productId)
    {
        const auto it = std::ranges::find_if(products_, [&](const Product &p)
        {
            return p.id() == productId;
        });

        if (it == products_.end())
        {
            return false;
        }

        products_.erase(it);
        return true;
    }

    double MonthlyList::plannedTotal() const
    {
        return std::accumulate(products_.begin(), products_.end(), 0.0,
                               [](const double acc, const Product &p)
                               {
                                   return acc + p.plannedTotal();
                               });
    }

    double MonthlyList::realTotal() const
    {
        return std::accumulate(products_.begin(), products_.end(), 0.0,
                               [](const double acc, const Product &p)
                               {
                                   return acc + p.realTotal();
                               });
    }

    double MonthlyList::savingsTotal() const
    {
        return std::accumulate(products_.begin(), products_.end(), 0.0,
                               [](const double acc, const Product &p)
                               {
                                   return acc + p.savings();
                               });
    }

    double MonthlyList::projectedMonthlyCost() const
    {
        return plannedTotal() - savingsTotal() + offBudgetAmount_;
    }
}
