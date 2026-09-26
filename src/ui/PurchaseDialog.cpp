#include "ui/PurchaseDialog.hpp"

#include "stapik/locale/LocaleManager.hpp"

namespace groceries::ui
{
    PurchaseDialog::PurchaseDialog(Gtk::Window &parent,
                                   const std::string &productName,
                                   const std::string &unitName,
                                   const MoneyFormatter &formatter,
                                   std::function<void(double, double)> onConfirm)
        : unitName_(unitName), formatter_(formatter)
    {
        const auto &loc = LocaleManager::instance();

        set_transient_for(parent);
        set_modal(true);
        set_default_size(340, -1);
        set_title(loc.translate("dialog.purchase.title"));

        rootBox_.set_margin(12);

        infoLabel_.set_text(loc.translate("dialog.purchase.info") + ": " + productName);
        infoLabel_.set_xalign(0.0f);
        rootBox_.append(infoLabel_);

        quantityCaption_.set_text(loc.translate("dialog.purchase.quantity.label") + " (" + unitName_ + ")");
        quantityCaption_.set_xalign(0.0f);
        quantityCaption_.set_width_chars(20);
        quantitySpin_.set_range(0.0001, 100000.0);
        quantitySpin_.set_increments(0.1, 1.0);
        quantitySpin_.set_digits(3);
        quantitySpin_.set_value(1.0);
        quantityRow_.append(quantityCaption_);
        quantitySpin_.set_hexpand(true);
        quantityRow_.append(quantitySpin_);
        rootBox_.append(quantityRow_);

        paidCaption_.set_text(loc.translate("dialog.purchase.paidTotal.label") + " (" +
                              formatter_.currency().symbol + ")");
        paidCaption_.set_xalign(0.0f);
        paidCaption_.set_width_chars(20);
        paidSpin_.set_range(0.0, 100000.0);
        paidSpin_.set_increments(0.1, 5.0);
        paidSpin_.set_digits(2);
        paidRow_.append(paidCaption_);
        paidSpin_.set_hexpand(true);
        paidRow_.append(paidSpin_);
        rootBox_.append(paidRow_);

        computedUnitPriceLabel_.set_xalign(0.0f);
        rootBox_.append(computedUnitPriceLabel_);
        updateComputedUnitPriceLabel();

        cancelButton_.set_label(loc.translate("dialog.button.cancel"));
        confirmButton_.set_label(loc.translate("dialog.purchase.button.save"));
        buttonBox_.set_halign(Gtk::Align::END);
        buttonBox_.append(cancelButton_);
        buttonBox_.append(confirmButton_);
        rootBox_.append(buttonBox_);

        rootBox_.add_css_class("groceries-window-frame");
        set_child(rootBox_);

        quantitySpin_.signal_value_changed().connect(
            sigc::mem_fun(*this, &PurchaseDialog::updateComputedUnitPriceLabel));
        paidSpin_.signal_value_changed().connect(
            sigc::mem_fun(*this, &PurchaseDialog::updateComputedUnitPriceLabel));

        cancelButton_.signal_clicked().connect([this]() { close(); });
        confirmButton_.signal_clicked().connect([this, onConfirm]()
        {
            const double quantity = quantitySpin_.get_value();
            const double totalPaid = paidSpin_.get_value();
            if (quantity <= 0.0)
            {
                quantitySpin_.grab_focus();
                return;
            }
            onConfirm(quantity, totalPaid);
            close();
        });

        present();
        signal_hide().connect([this]() { delete this; });
    }

    void PurchaseDialog::updateComputedUnitPriceLabel()
    {
        const double quantity = quantitySpin_.get_value();
        const double totalPaid = paidSpin_.get_value();
        const std::string price =
                quantity > 0.0 ? formatter_.format(totalPaid / quantity) : formatter_.placeholder();
        computedUnitPriceLabel_.set_text("= " + price + " / " + unitName_);
    }
}
