#pragma once

#include "core/GroceriesDocument.hpp"
#include "domain/MonthlyList.hpp"

namespace groceries::core
{
    class MonthCoordinator
    {
    public:
        explicit MonthCoordinator(GroceriesDocument &document);
        domain::MonthlyList &startMonth(domain::MonthId target, const domain::MonthId *sourceMonth = nullptr) const;
        [[nodiscard]] domain::MonthlyList &startNextMonth() const;
    private:
        GroceriesDocument &document_;
    };
}
