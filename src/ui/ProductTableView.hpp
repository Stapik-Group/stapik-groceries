#pragma once

#include <functional>
#include <string>
#include <vector>

#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>

#include "domain/Category.hpp"
#include "domain/MonthlyList.hpp"
#include "ui/MoneyFormatter.hpp"

namespace groceries::ui
{
    class ProductTableView : public Gtk::ScrolledWindow
    {
    public:
        explicit ProductTableView(const MoneyFormatter &formatter);

        void refresh(const domain::MonthlyList &month, const std::vector<domain::Category> &categories);

        std::function<void(const std::string &productId)> onRecordPurchase;
        std::function<void(const std::string &productId)> onEditProduct;
        std::function<void(const std::string &productId)> onDeleteProduct;

    private:
        const MoneyFormatter &formatter_;

        Gtk::ListBox listBox_;
        const domain::MonthlyList *lastMonth_ = nullptr;
        const std::vector<domain::Category> *lastCategories_ = nullptr;

        static const domain::Category *findCategory(const std::vector<domain::Category> &categories, const std::string &categoryId);
    };
}
