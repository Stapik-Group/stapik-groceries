#pragma once

#include <string>
#include <vector>

namespace groceries::domain
{
    struct Currency
    {
        std::string code;
        std::string symbol;
        bool symbolBeforeAmount;
    };

    class CurrencyCatalog
    {
    public:
        static const std::vector<Currency> &all();
        static const Currency &defaultCurrency();
        static const Currency &findByCode(const std::string &code);
    };
}
