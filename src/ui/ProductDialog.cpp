#include "ui/ProductDialog.hpp"

#include <algorithm>
#include <cctype>

#include <gtkmm/label.h>

#include "stapik/locale/LocaleManager.hpp"

namespace groceries::ui
{
    using domain::Category;
    using domain::Product;

    namespace
    {
        Gtk::Box *labeledRow(const std::string &labelText, Gtk::Widget &widget, const Glib::RefPtr<Gtk::SizeGroup> &sizeGroup)
        {
            auto row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 8);
            auto label = Gtk::make_managed<Gtk::Label>(labelText);
            label->set_xalign(0.0f);
            sizeGroup->add_widget(*label);
            row->append(*label);
            widget.set_hexpand(true);
            row->append(widget);
            return row;
        }
    }

    ProductDialog::ProductDialog(Window &parent,
                                 const std::vector<Category> &categories,
                                 const Product *existingProduct,
                                 std::vector<std::string> existingProductNames,
                                 std::function<void(const ProductFormValues &)> onSave)
        : existingProductNames_(std::move(existingProductNames))
    {
        const auto &loc = LocaleManager::instance();

        set_transient_for(parent);
        set_modal(true);
        set_default_size(360, -1);
        set_title(existingProduct == nullptr
                      ? loc.translate("dialog.product.new.title")
                      : loc.translate("dialog.product.edit.title"));

        std::vector<Glib::ustring> categoryNames;
        for (const Category &category: categories)
        {
            categoryIds_.push_back(category.id());
            categoryNames.push_back(category.name());
        }
        categoryModel_ = Gtk::StringList::create(categoryNames);
        categoryDropDown_.set_model(categoryModel_);

        rootBox_.set_margin(12);
        rootBox_.append(*labeledRow(loc.translate("dialog.product.name.label"), nameEntry_, labelSizeGroup_));
        rootBox_.append(*labeledRow(loc.translate("dialog.product.category.label"), categoryDropDown_,
                                    labelSizeGroup_));
        rootBox_.append(*labeledRow(loc.translate("dialog.product.unit.label"), unitEntry_, labelSizeGroup_));

        quantitySpin_.set_range(0.0, 1000.0);
        quantitySpin_.set_increments(1.0, 5.0);
        quantitySpin_.set_digits(2);
        rootBox_.append(*labeledRow(loc.translate("dialog.product.plannedQuantity.label"), quantitySpin_,
                                    labelSizeGroup_));

        priceSpin_.set_range(0.0, 100000.0);
        priceSpin_.set_increments(0.5, 5.0);
        priceSpin_.set_digits(2);
        rootBox_.append(*labeledRow(loc.translate("dialog.product.plannedPrice.label"), priceSpin_, labelSizeGroup_));

        if (existingProduct != nullptr)
        {
            nameEntry_.set_text(existingProduct->name());
            unitEntry_.set_text(existingProduct->unitName());
            quantitySpin_.set_value(existingProduct->plannedQuantity());
            priceSpin_.set_value(existingProduct->plannedUnitPrice());

            for (std::size_t i = 0; i < categoryIds_.size(); ++i)
            {
                if (categoryIds_[i] == existingProduct->categoryId())
                {
                    categoryDropDown_.set_selected(static_cast<guint>(i));
                    break;
                }
            }
        }

        cancelButton_.set_label(loc.translate("dialog.button.cancel"));
        saveButton_.set_label(loc.translate("dialog.button.save"));
        buttonBox_.set_halign(Gtk::Align::END);
        buttonBox_.append(cancelButton_);
        buttonBox_.append(saveButton_);

        errorLabel_.set_xalign(0.0f);
        errorLabel_.add_css_class("error");
        errorLabel_.set_visible(false);
        rootBox_.append(errorLabel_);
        rootBox_.append(buttonBox_);

        rootBox_.add_css_class("groceries-window-frame");
        set_child(rootBox_);

        cancelButton_.signal_clicked().connect([this]() { close(); });
        saveButton_.signal_clicked().connect(
            sigc::bind(sigc::mem_fun(*this, &ProductDialog::onSaveClicked), onSave));

        present();
        signal_hide().connect([this]() { delete this; });
    }

    void ProductDialog::onSaveClicked(std::function<void(const ProductFormValues &)> onSave)
    {
        ProductFormValues values;
        values.name = nameEntry_.get_text();
        values.unitName = unitEntry_.get_text();
        values.plannedQuantity = quantitySpin_.get_value();
        values.plannedUnitPrice = priceSpin_.get_value();

        const guint selected = categoryDropDown_.get_selected();
        if (selected != GTK_INVALID_LIST_POSITION && selected < categoryIds_.size())
        {
            values.categoryId = categoryIds_[selected];
        }

        if (values.name.empty())
        {
            errorLabel_.set_text(LocaleManager::instance().translate("dialog.product.error.nameRequired"));
            errorLabel_.set_visible(true);
            nameEntry_.grab_focus();
            return;
        }

        auto sameNameIgnoreCase = [&values](const std::string &other)
        {
            if (other.size() != values.name.size())
            {
                return false;
            }
            return std::equal(other.begin(), other.end(), values.name.begin(), [](char a, char b)
            {
                return std::tolower(static_cast<unsigned char>(a)) ==
                       std::tolower(static_cast<unsigned char>(b));
            });
        };
        if (std::any_of(existingProductNames_.begin(), existingProductNames_.end(), sameNameIgnoreCase))
        {
            errorLabel_.set_text(LocaleManager::instance().translate("dialog.product.error.duplicateName"));
            errorLabel_.set_visible(true);
            nameEntry_.grab_focus();
            return;
        }

        onSave(values);
        close();
    }
}
