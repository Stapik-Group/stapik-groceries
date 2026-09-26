#include "domain/Category.hpp"

#include <utility>

namespace groceries::domain
{
    Category::Category(std::string id, std::string name, std::string colorHex)
        : id_(std::move(id)), name_(std::move(name)), colorHex_(std::move(colorHex))
    {
    }

    void Category::rename(std::string newName)
    {
        name_ = std::move(newName);
    }

    void Category::recolor(std::string newColorHex)
    {
        colorHex_ = std::move(newColorHex);
    }
}
