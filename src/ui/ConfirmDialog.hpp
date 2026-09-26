#pragma once

#include <functional>
#include <string>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>

namespace groceries::ui
{
    class ConfirmDialog : public Gtk::Window
    {
    public:
        ConfirmDialog(Window &parent,
                      const std::string &title,
                      const std::string &message,
                      const std::string &confirmLabel,
                      std::function<void()> onConfirmed);

    private:
        Gtk::Box rootBox_{Gtk::Orientation::VERTICAL, 12};
        Gtk::Label messageLabel_;
        Gtk::Box buttonBox_{Gtk::Orientation::HORIZONTAL, 8};
        Gtk::Button confirmButton_;
        Gtk::Button cancelButton_;
    };
}
