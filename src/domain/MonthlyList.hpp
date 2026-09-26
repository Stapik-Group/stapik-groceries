#pragma once

#include <string>
#include <vector>

#include "domain/Product.hpp"

namespace groceries::domain
{
    struct MonthId
    {
        int year = 0;
        int month = 1;

        [[nodiscard]] std::string toKey() const;
        static MonthId fromKey(const std::string &key);

        bool operator==(const MonthId &other) const
        {
            return year == other.year && month == other.month;
        }

        bool operator<(const MonthId &other) const
        {
            return toKey() < other.toKey();
        }
    };

    class MonthlyList
    {
    public:
        explicit MonthlyList(MonthId month);

        [[nodiscard]] const MonthId &month() const { return month_; }

        [[nodiscard]] const std::vector<Product> &products() const { return products_; }
        std::vector<Product> &products() { return products_; }

        Product *findProduct(const std::string &productId);

        void addProduct(Product product);

        bool removeProduct(const std::string &productId);

        [[nodiscard]] double offBudgetAmount() const { return offBudgetAmount_; }
        void setOffBudgetAmount(double amount) { offBudgetAmount_ = amount; }

        [[nodiscard]] double plannedTotal() const;
        [[nodiscard]] double realTotal() const;
        [[nodiscard]] double savingsTotal() const;
        [[nodiscard]] double projectedMonthlyCost() const;

    private:
        MonthId month_;
        std::vector<Product> products_;
        double offBudgetAmount_ = 0.0;
    };
}