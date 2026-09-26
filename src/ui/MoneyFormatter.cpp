#include "ui/MoneyFormatter.hpp"

#include <cstdio>

namespace groceries::ui
{
    std::string MoneyFormatter::format(double amount) const
    {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%.2f", amount);
        return compose(buffer);
    }

    std::string MoneyFormatter::placeholder() const
    {
        return compose("—");
    }

    std::string MoneyFormatter::compose(const std::string &number) const
    {
        return currency_->symbolBeforeAmount
                   ? currency_->symbol + number
                   : number + " " + currency_->symbol;
    }
}
