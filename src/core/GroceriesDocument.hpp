#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "domain/Category.hpp"
#include "domain/Currency.hpp"
#include "domain/MonthlyList.hpp"

namespace groceries::core
{
    class GroceriesDocument
    {
    public:
        domain::Category *findCategory(const std::string &id);
        domain::MonthlyList *findMonth(const domain::MonthId &id);
        domain::MonthlyList &upsertMonth(domain::MonthId id);

        void addCategory(domain::Category category);
        void setCurrencyCode(std::string code) { currencyCode_ = std::move(code); }
        void touch(const std::int64_t nowEpochMs) { lastUpdateEpochMs_ = nowEpochMs; }
        void setLastKnownCloudUpdateEpochMs(const std::optional<std::int64_t> value) { lastKnownCloudUpdateEpochMs_ = value; }

        bool removeCategory(const std::string &id);
        bool removeMonth(const domain::MonthId &id);

        [[nodiscard]] const std::vector<domain::Category> &categories() const { return categories_; }
        [[nodiscard]] const std::vector<domain::MonthlyList> &months() const { return months_; }
        [[nodiscard]] std::optional<domain::MonthId> latestMonth() const;
        [[nodiscard]] std::int64_t lastUpdateEpochMs() const { return lastUpdateEpochMs_; }
        [[nodiscard]] const std::string &currencyCode() const { return currencyCode_; }
        [[nodiscard]] std::optional<std::int64_t> lastKnownCloudUpdateEpochMs() const { return lastKnownCloudUpdateEpochMs_; }
    private:
        std::vector<domain::Category> categories_;
        std::vector<domain::MonthlyList> months_;
        std::string currencyCode_ = domain::CurrencyCatalog::defaultCurrency().code;
        std::int64_t lastUpdateEpochMs_ = 0;
        std::optional<std::int64_t> lastKnownCloudUpdateEpochMs_;
    };
}
