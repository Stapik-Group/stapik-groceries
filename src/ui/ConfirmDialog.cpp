#include "ui/ConfirmDialog.hpp"

#include "stapik/locale/LocaleManager.hpp"

namespace groceries::ui
{
    ConfirmDialog::ConfirmDialog(Window &parent,
                                 const std::string &title,
                                 const std::string &message,
                                 const std::string &confirmLabel,
                                 std::function<void()> onConfirmed)
        : confirmButton_(confirmLabel)
    {
        set_transient_for(parent);
        set_modal(true);
        set_default_size(340, -1);
        set_title(title);

        rootBox_.set_margin(12);

        messageLabel_.set_text(message);
        messageLabel_.set_wrap(true);
        messageLabel_.set_xalign(0.0f);
        rootBox_.append(messageLabel_);

        cancelButton_.set_label(LocaleManager::instance().translate("dialog.button.cancel"));
        confirmButton_.add_css_class("destructive-action");

        buttonBox_.set_halign(Gtk::Align::END);
        buttonBox_.append(cancelButton_);
        buttonBox_.append(confirmButton_);
        rootBox_.append(buttonBox_);

        rootBox_.add_css_class("groceries-window-frame");
        set_child(rootBox_);

        cancelButton_.signal_clicked().connect([this] { close(); });
        confirmButton_.signal_clicked().connect([this, onConfirmed]
        {
            onConfirmed();
            close();
        });

        present();
        signal_hide().connect([this] { delete this; });
    }
}
