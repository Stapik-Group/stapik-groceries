#pragma once

#include <functional>
#include <string>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/window.h>

#include "ui/MoneyFormatter.hpp"

namespace groceries::ui
{
    class PurchaseDialog : public Gtk::Window
    {
    public:
        PurchaseDialog(Gtk::Window &parent,
                       const std::string &productName,
                       const std::string &unitName,
                       const MoneyFormatter &formatter,
                       std::function<void(double quantity, double totalPaid)> onConfirm);

    private:
        std::string unitName_;
        const MoneyFormatter &formatter_;

        Gtk::Box rootBox_{Gtk::Orientation::VERTICAL, 8};
        Gtk::Label infoLabel_;
        Gtk::Box quantityRow_{Gtk::Orientation::HORIZONTAL, 8};
        Gtk::Label quantityCaption_;
        Gtk::SpinButton quantitySpin_;
        Gtk::Box paidRow_{Gtk::Orientation::HORIZONTAL, 8};
        Gtk::Label paidCaption_;
        Gtk::SpinButton paidSpin_;
        Gtk::Label computedUnitPriceLabel_;
        Gtk::Box buttonBox_{Gtk::Orientation::HORIZONTAL, 8};
        Gtk::Button confirmButton_;
        Gtk::Button cancelButton_;

        void updateComputedUnitPriceLabel();
    };
}
