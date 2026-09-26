#pragma once

#include <functional>
#include <string>
#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/dropdown.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/stringlist.h>
#include <gtkmm/window.h>
#include <gtkmm/sizegroup.h>

#include "domain/Category.hpp"
#include "domain/Product.hpp"

namespace groceries::ui
{
    struct ProductFormValues
    {
        std::string name;
        std::string categoryId;
        std::string unitName;
        double plannedQuantity = 1.0;
        double plannedUnitPrice = 0.0;
    };

    class ProductDialog : public Gtk::Window
    {
    public:
        ProductDialog(Window &parent,
                      const std::vector<domain::Category> &categories,
                      const domain::Product *existingProduct,
                      std::vector<std::string> existingProductNames,
                      std::function<void(const ProductFormValues &)> onSave);

    private:
        std::vector<std::string> categoryIds_;
        Glib::RefPtr<Gtk::StringList> categoryModel_;
        std::vector<std::string> existingProductNames_;

        Gtk::Box rootBox_{Gtk::Orientation::VERTICAL, 8};
        Glib::RefPtr<Gtk::SizeGroup> labelSizeGroup_ = Gtk::SizeGroup::create(Gtk::SizeGroup::Mode::HORIZONTAL);
        Gtk::Entry nameEntry_;
        Gtk::DropDown categoryDropDown_;
        Gtk::Entry unitEntry_;
        Gtk::SpinButton quantitySpin_;
        Gtk::SpinButton priceSpin_;
        Gtk::Label errorLabel_;
        Gtk::Box buttonBox_{Gtk::Orientation::HORIZONTAL, 8};
        Gtk::Button saveButton_;
        Gtk::Button cancelButton_;

        void onSaveClicked(std::function<void(const ProductFormValues &)> onSave);
    };
}
