#pragma once

#include <string>

namespace groceries::domain
{
    class Category
    {
    public:
        Category() = default;
        Category(std::string id, std::string name, std::string colorHex);

        void rename(std::string newName);
        void recolor(std::string newColorHex);

        [[nodiscard]] const std::string &id() const { return id_; }
        [[nodiscard]] const std::string &name() const { return name_; }
        [[nodiscard]] const std::string &colorHex() const { return colorHex_; }
    private:
        std::string id_;
        std::string name_;
        std::string colorHex_ = "#CCCCCC";
    };
}
