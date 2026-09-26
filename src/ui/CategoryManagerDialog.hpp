#pragma once

#include <functional>
#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

#include "domain/Category.hpp"

namespace groceries::ui
{
    struct CategoryManagerCallbacks
    {
        std::function<domain::Category(const std::string &name)> onAdd;
        std::function<void(const std::string &id, const std::string &newName)> onRename;
        std::function<void(const std::string &id, const std::string &newColorHex)> onRecolor;
        std::function<void(const std::string &id)> onRemove;
    };

    class CategoryManagerDialog : public Gtk::Window
    {
    public:
        CategoryManagerDialog(Window &parent,
                              std::vector<domain::Category> categories,
                              CategoryManagerCallbacks callbacks);

    private:
        void addCategory();

        std::vector<domain::Category> categories_;
        CategoryManagerCallbacks callbacks_;

        Gtk::Box rootBox_{Gtk::Orientation::VERTICAL, 8};
        Gtk::ScrolledWindow scroller_;
        Gtk::ListBox listBox_;
        Gtk::Box addRow_{Gtk::Orientation::HORIZONTAL, 8};
        Gtk::Entry newCategoryEntry_;
        Gtk::Button addButton_;
        Gtk::Label addErrorLabel_;
        Gtk::Button closeButton_;

        void rebuildRows();

        void appendRowFor(const domain::Category &category);
    };
}