#include "ui/ProductTableView.hpp"

#include <algorithm>
#include <cstdio>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/label.h>

#include "stapik/locale/LocaleManager.hpp"

namespace groceries::ui
{
    using domain::Category;
    using domain::MonthlyList;
    using domain::Product;

    namespace
    {
        std::string formatQuantity(double value)
        {
            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "%.2g", value);
            return std::string(buffer);
        }

        Gtk::Widget *makeCategoryChip(const std::string &colorHex)
        {
            auto chip = Gtk::make_managed<Gtk::Box>();
            chip->add_css_class("groceries-category-chip");
            chip->set_valign(Gtk::Align::CENTER);

            auto provider = Gtk::CssProvider::create();
            const std::string css = "box { background-color: " + colorHex + "; }";
            provider->load_from_data(css);
            chip->get_style_context()->add_provider(provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

            return chip;
        }
    }

    ProductTableView::ProductTableView(const MoneyFormatter &formatter) : formatter_(formatter)
    {
        set_hexpand(true);
        set_vexpand(true);
        add_css_class("groceries-table");
        set_child(listBox_);
        listBox_.add_css_class("groceries-list");
        listBox_.set_selection_mode(Gtk::SelectionMode::NONE);

        LocaleManager::instance().signalLocaleChanged().connect([this]
        {
            if (lastMonth_ != nullptr && lastCategories_ != nullptr)
            {
                refresh(*lastMonth_, *lastCategories_);
            }
        });
    }

    const Category *ProductTableView::findCategory(const std::vector<Category> &categories,
                                                   const std::string &categoryId)
    {
        auto it = std::find_if(categories.begin(), categories.end(),
                               [&](const Category &c) { return c.id() == categoryId; });
        return it == categories.end() ? nullptr : &(*it);
    }

    void ProductTableView::refresh(const MonthlyList &month, const std::vector<Category> &categories)
    {
        lastMonth_ = &month;
        lastCategories_ = &categories;

        const auto &loc = LocaleManager::instance();

        while (Gtk::Widget *child = listBox_.get_first_child())
        {
            listBox_.remove(*child);
        }

        for (const Product &product: month.products())
        {
            auto row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 4);
            row->add_css_class("groceries-row");
            auto topLine = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 12);
            auto detailsLine = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 16);

            auto nameLabel = Gtk::make_managed<Gtk::Label>(product.name());
            nameLabel->add_css_class("groceries-product-name");
            nameLabel->set_width_chars(20);
            nameLabel->set_max_width_chars(20);
            nameLabel->set_ellipsize(Pango::EllipsizeMode::END);
            nameLabel->set_xalign(0.0f);

            const Category *category = findCategory(categories, product.categoryId());

            auto categoryBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 4);
            categoryBox->set_size_request(140, -1);
            if (category != nullptr)
            {
                categoryBox->append(*makeCategoryChip(category->colorHex()));
            }
            auto categoryLabel = Gtk::make_managed<Gtk::Label>(category != nullptr
                                                      ? category->name()
                                                      : loc.translate("category.none"));
            categoryLabel->set_xalign(0.0f);
            categoryBox->append(*categoryLabel);

            auto planLabel = Gtk::make_managed<Gtk::Label>(
                formatQuantity(product.plannedQuantity()) + " " + product.unitName() + " x " +
                formatter_.format(product.plannedUnitPrice()) + " = " + formatter_.format(product.plannedTotal()));
            planLabel->set_xalign(0.0f);

            auto realLabel = Gtk::make_managed<Gtk::Label>(
                loc.translate("table.purchased") + ": " + formatQuantity(product.purchasedQuantity()) + " " +
                product.unitName() + " (" + loc.translate("table.average") + " " +
                formatter_.format(product.realAverageUnitPrice()) +
                "/" + product.unitName() + ") = " + formatter_.format(product.realTotal()));
            realLabel->set_xalign(0.0f);

            auto savingsLabel = Gtk::make_managed<Gtk::Label>(
                loc.translate("table.savings") + ": " + formatter_.format(product.savings()));
            savingsLabel->set_xalign(0.0f);
            savingsLabel->add_css_class(product.savings() > 0.0
                                            ? "groceries-savings-positive"
                                            : "groceries-savings-zero");

            topLine->append(*nameLabel);
            topLine->append(*categoryBox);
            detailsLine->append(*planLabel);
            detailsLine->append(*realLabel);
            detailsLine->append(*savingsLabel);

            const std::string productId = product.id();

            auto actionsBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 8);
            actionsBox->add_css_class("groceries-row-actions");
            actionsBox->set_hexpand(true);
            actionsBox->set_halign(Gtk::Align::END);

            auto purchaseButton = Gtk::make_managed<Gtk::Button>(loc.translate("action.bought"));
            purchaseButton->signal_clicked().connect([this, productId]()
            {
                if (onRecordPurchase)
                {
                    onRecordPurchase(productId);
                }
            });
            actionsBox->append(*purchaseButton);

            auto editButton = Gtk::make_managed<Gtk::Button>(loc.translate("action.edit"));
            editButton->signal_clicked().connect([this, productId]()
            {
                if (onEditProduct)
                {
                    onEditProduct(productId);
                }
            });
            actionsBox->append(*editButton);

            auto deleteButton = Gtk::make_managed<Gtk::Button>(loc.translate("action.delete"));
            deleteButton->signal_clicked().connect([this, productId]()
            {
                if (onDeleteProduct)
                {
                    onDeleteProduct(productId);
                }
            });
            actionsBox->append(*deleteButton);

            topLine->append(*actionsBox);

            row->append(*topLine);
            row->append(*detailsLine);

            listBox_.append(*row);
        }
    }
}
