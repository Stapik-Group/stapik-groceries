#include "domain/Currency.hpp"

namespace groceries::domain
{
    const std::vector<Currency> &CurrencyCatalog::all()
    {
        static const std::vector<Currency> currencies = {
            {.code = "PLN", .symbol = "zł", .symbolBeforeAmount = false},
            {.code = "EUR", .symbol = "€", .symbolBeforeAmount = false},
            {.code = "USD", .symbol = "$", .symbolBeforeAmount = true},
            {.code = "GBP", .symbol = "£", .symbolBeforeAmount = true},
            {.code = "CZK", .symbol = "Kč", .symbolBeforeAmount = false},
            {.code = "CHF", .symbol = "CHF", .symbolBeforeAmount = false},
        };
        return currencies;
    }

    const Currency &CurrencyCatalog::defaultCurrency()
    {
        return all().front();
    }

    const Currency &CurrencyCatalog::findByCode(const std::string &code)
    {
        for (const Currency &currency: all())
        {
            if (currency.code == code)
            {
                return currency;
            }
        }
        return defaultCurrency();
    }
}