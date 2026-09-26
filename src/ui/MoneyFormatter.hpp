#pragma once

#include <string>

#include "domain/Currency.hpp"

namespace groceries::ui
{
    class MoneyFormatter
    {
    public:
        explicit MoneyFormatter(const domain::Currency &currency = domain::CurrencyCatalog::defaultCurrency())
            : currency_(&currency)
        {
        }

        void setCurrency(const domain::Currency &currency) { currency_ = &currency; }
        const domain::Currency &currency() const { return *currency_; }

        std::string format(double amount) const;
        std::string placeholder() const;

    private:
        const domain::Currency *currency_;

        std::string compose(const std::string &number) const;
    };
}
