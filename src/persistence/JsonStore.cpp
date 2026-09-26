#include "persistence/JsonStore.hpp"

#include <fstream>
#include <stdexcept>

#include "AppInfo.hpp"
#include "stapik/storage/AppPaths.hpp"

namespace groceries::persistence
{
    using core::GroceriesDocument;
    using domain::Category;
    using domain::MonthId;
    using domain::MonthlyList;
    using domain::Product;
    using json = nlohmann::json;

    namespace
    {
        json productToJson(const Product &product)
        {
            return json{
                {"id", product.id()},
                {"name", product.name()},
                {"categoryId", product.categoryId()},
                {"unitName", product.unitName()},
                {"plannedQuantity", product.plannedQuantity()},
                {"plannedUnitPrice", product.plannedUnitPrice()},
                {"purchasedQuantity", product.purchasedQuantity()},
                {"realAverageUnitPrice", product.realAverageUnitPrice()},
            };
        }

        Product productFromJson(const json &j)
        {
            Product product(j.at("id").get<std::string>(),
                            j.at("name").get<std::string>(),
                            j.at("categoryId").get<std::string>(),
                            j.at("unitName").get<std::string>(),
                            j.at("plannedQuantity").get<double>(),
                            j.at("plannedUnitPrice").get<double>());

            const double purchasedQuantity =
                    j.contains("purchasedQuantity")
                        ? j.at("purchasedQuantity").get<double>()
                        : static_cast<double>(j.value("timesPurchased", 0));
            const double avgPrice = j.value("realAverageUnitPrice", 0.0);
            product.restorePurchaseState(purchasedQuantity, avgPrice);

            return product;
        }

        json categoryToJson(const Category &category)
        {
            return json{
                {"id", category.id()},
                {"name", category.name()},
                {"colorHex", category.colorHex()},
            };
        }

        Category categoryFromJson(const json &j)
        {
            return {j.at("id").get<std::string>(),
                            j.at("name").get<std::string>(),
                            j.value("colorHex", std::string("#CCCCCC"))};
        }

        json monthToJson(const MonthlyList &month)
        {
            json products = json::array();
            for (const Product &product: month.products())
            {
                products.push_back(productToJson(product));
            }
            return json{
                {"month", month.month().toKey()},
                {"offBudgetAmount", month.offBudgetAmount()},
                {"products", std::move(products)},
            };
        }

        MonthlyList monthFromJson(const json &j)
        {
            MonthlyList month(MonthId::fromKey(j.at("month").get<std::string>()));
            month.setOffBudgetAmount(j.value("offBudgetAmount", 0.0));
            for (const auto &productJson: j.value("products", json::array()))
            {
                month.addProduct(productFromJson(productJson));
            }
            return month;
        }
    }

    json JsonStore::toJson(const GroceriesDocument &document)
    {
        json categories = json::array();
        for (const Category &category: document.categories())
        {
            categories.push_back(categoryToJson(category));
        }

        json months = json::array();
        for (const MonthlyList &month: document.months())
        {
            months.push_back(monthToJson(month));
        }

        json result = {
            {"formatVersion", 1},
            {"lastUpdate", document.lastUpdateEpochMs()},
            {"currency", document.currencyCode()},
            {"categories", std::move(categories)},
            {"months", std::move(months)},
        };
        if (document.lastKnownCloudUpdateEpochMs().has_value())
        {
            result["lastKnownCloudUpdate"] = *document.lastKnownCloudUpdateEpochMs();
        }
        return result;
    }

    GroceriesDocument JsonStore::fromJson(const json &json)
    {
        GroceriesDocument document;

        document.setCurrencyCode(
            json.value("currency", domain::CurrencyCatalog::defaultCurrency().code));
        if (json.contains("lastKnownCloudUpdate"))
        {
            document.setLastKnownCloudUpdateEpochMs(json.at("lastKnownCloudUpdate").get<std::int64_t>());
        }

        for (const auto &categoryJson: json.value("categories", json::array()))
        {
            document.addCategory(categoryFromJson(categoryJson));
        }

        for (const auto &monthJson: json.value("months", json::array()))
        {
            MonthlyList month = monthFromJson(monthJson);
            const MonthId id = month.month();
            MonthlyList &target = document.upsertMonth(id);
            target = std::move(month);
        }

        document.touch(json.value("lastUpdate", static_cast<std::int64_t>(0)));

        return document;
    }

    std::filesystem::path JsonStore::defaultDataFilePath()
    {
        return AppPaths::userDataDir(APP_NAME) / GROCERIES_FILENAME;
    }

    void JsonStore::saveToFile(const GroceriesDocument &document, const std::filesystem::path &path)
    {
        std::filesystem::create_directories(path.parent_path());

        const json j = toJson(document);

        const std::filesystem::path tmpPath = path.string() + ".tmp";
        {
            std::ofstream out(tmpPath);
            if (!out)
            {
                throw std::runtime_error("Can't load read-only file: " + tmpPath.string());
            }
            out << j.dump(2);
        }
        std::filesystem::rename(tmpPath, path);
    }

    GroceriesDocument JsonStore::loadFromFile(const std::filesystem::path &path)
    {
        if (!std::filesystem::exists(path))
        {
            return GroceriesDocument{};
        }

        std::ifstream in(path);
        if (!in)
        {
            throw std::runtime_error("Can't load read-only file: " + path.string());
        }

        json j;
        in >> j;
        return fromJson(j);
    }
}