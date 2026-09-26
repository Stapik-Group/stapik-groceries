#pragma once

#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>

#include "core/GroceriesDocument.hpp"

namespace groceries::persistence {

// Odpowiada wyłącznie za (de)serializację GroceriesDocument <-> JSON oraz
// odczyt/zapis na dysk. Format zgodny z podejściem stapik-budgeting: jeden
// dokument JSON z lastUpdate do rozstrzygania konfliktów przy cloud sync.
class JsonStore {
public:
    static nlohmann::json toJson(const core::GroceriesDocument& document);
    static core::GroceriesDocument fromJson(const nlohmann::json& json);

    // Ścieżka domyślna: ~/.local/share/stapikgroceries/groceries.json
    static std::filesystem::path defaultDataFilePath();

    static void saveToFile(const core::GroceriesDocument& document,
                            const std::filesystem::path& path);
    static core::GroceriesDocument loadFromFile(const std::filesystem::path& path);
};

} // namespace groceries::persistence
