#include "core/MonthCoordinator.hpp"

namespace groceries::core
{
    using domain::MonthId;
    using domain::MonthlyList;
    using domain::Product;

    MonthCoordinator::MonthCoordinator(GroceriesDocument &document) : document_(document) { }

    MonthlyList &MonthCoordinator::startMonth(const MonthId target, const MonthId *sourceMonth) const
    {
        if (auto *existing = document_.findMonth(target))
        {
            return *existing;
        }

        MonthlyList &created = document_.upsertMonth(target);

        if (sourceMonth != nullptr)
        {
            if (const MonthlyList *source = document_.findMonth(*sourceMonth))
            {
                for (const Product &sourceProduct: source->products())
                {
                    Product copy = sourceProduct;
                    copy.resetPurchasesForNewMonth();
                    created.addProduct(std::move(copy));
                }
            }
        }

        return created;
    }

    MonthlyList &MonthCoordinator::startNextMonth() const
    {
        const auto latest = document_.latestMonth();
        if (!latest.has_value())
        {
            constexpr MonthId now{};
            return document_.upsertMonth(now);
        }

        MonthId next = *latest;
        next.month += 1;
        if (next.month > 12)
        {
            next.month = 1;
            next.year += 1;
        }

        return startMonth(next, &*latest);
    }
}