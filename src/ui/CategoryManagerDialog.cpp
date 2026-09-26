#include "ui/CategoryManagerDialog.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <memory>

#include <gdkmm/rgba.h>
#include "stapik/locale/LocaleManager.hpp"
#include "ui/ConfirmDialog.hpp"

namespace groceries::ui
{
    using domain::Category;

    namespace
    {
        Gdk::RGBA hexToRgba(const std::string &hex)
        {
            Gdk::RGBA rgba;
            rgba.set(hex);
            return rgba;
        }

        std::string rgbaToHex(const Gdk::RGBA &rgba)
        {
            const auto to255 = [](const float component)
            {
                return static_cast<int>(std::lround(component * 255.0f));
            };

            return std::format("#{:02X}{:02X}{:02X}",
                               to255(rgba.get_red()),
                               to255(rgba.get_green()),
                               to255(rgba.get_blue()));
        }

        bool sameNameIgnoreCase(const Category &category, const std::string &name)
        {
            if (category.name().size() != name.size())
            {
                return false;
            }

            return std::ranges::equal(category.name(), name, [](const char a, const char b)
            {
                return std::tolower(static_cast<unsigned char>(a)) ==
                       std::tolower(static_cast<unsigned char>(b));
            });
        }
    }

    CategoryManagerDialog::CategoryManagerDialog(Window &parent,
                                                 std::vector<Category> categories,
                                                 CategoryManagerCallbacks callbacks)
        : categories_(std::move(categories)), callbacks_(std::move(callbacks))
    {
        const auto &loc = LocaleManager::instance();

        set_transient_for(parent);
        set_modal(true);
        set_default_size(420, 360);
        set_title(loc.translate("dialog.categories.title"));

        rootBox_.set_margin(12);

        scroller_.set_child(listBox_);
        scroller_.set_vexpand(true);
        listBox_.set_selection_mode(Gtk::SelectionMode::NONE);
        rootBox_.append(scroller_);

        newCategoryEntry_.set_hexpand(true);
        newCategoryEntry_.set_placeholder_text(loc.translate("dialog.categories.newName.placeholder"));
        addButton_.set_label(loc.translate("dialog.categories.button.add"));
        addRow_.append(newCategoryEntry_);
        addRow_.append(addButton_);
        rootBox_.append(addRow_);

        addErrorLabel_.set_xalign(0.0f);
        addErrorLabel_.add_css_class("error");
        addErrorLabel_.set_visible(false);
        rootBox_.append(addErrorLabel_);

        closeButton_.set_label(loc.translate("dialog.categories.button.close"));
        closeButton_.set_halign(Gtk::Align::END);
        rootBox_.append(closeButton_);

        rootBox_.add_css_class("groceries-window-frame");
        set_child(rootBox_);

        addButton_.signal_clicked().connect([this]
        {
            addCategory();
        });

        closeButton_.signal_clicked().connect([this]
        {
            close();
        });

        rebuildRows();
        present();
        signal_hide().connect([this]
        {
            delete this;
        });
    }

    void CategoryManagerDialog::addCategory()
    {
        const std::string name = newCategoryEntry_.get_text();
        if (name.empty())
        {
            return;
        }

        if (std::ranges::any_of(categories_, [&](const Category &category)
        {
            return sameNameIgnoreCase(category, name);
        }))
        {
            addErrorLabel_.set_text(
                LocaleManager::instance().translate("dialog.categories.error.duplicateName"));
            addErrorLabel_.set_visible(true);
            return;
        }

        addErrorLabel_.set_visible(false);

        if (!callbacks_.onAdd)
        {
            return;
        }

        Category created = callbacks_.onAdd(name);
        categories_.push_back(std::move(created));
        appendRowFor(categories_.back());
        newCategoryEntry_.set_text("");
    }

    void CategoryManagerDialog::rebuildRows()
    {
        while (Widget *child = listBox_.get_first_child())
        {
            listBox_.remove(*child);
        }

        for (const Category &category: categories_)
        {
            appendRowFor(category);
        }
    }

    void CategoryManagerDialog::appendRowFor(const Category &category)
    {
        const auto &id = category.id();

        auto row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 8);
        row->set_margin(4);

        auto nameEntry = Gtk::make_managed<Gtk::Entry>();
        nameEntry->set_text(category.name());
        nameEntry->set_hexpand(true);

        auto colorButton = Gtk::make_managed<Gtk::ColorButton>();
        colorButton->set_rgba(hexToRgba(category.colorHex()));

        const auto removeButton = Gtk::make_managed<Gtk::Button>(
            LocaleManager::instance().translate("action.delete"));

        row->append(*nameEntry);
        row->append(*colorButton);
        row->append(*removeButton);
        listBox_.append(*row);

        nameEntry->signal_changed().connect([this, id, nameEntry]
        {
            if (callbacks_.onRename)
            {
                callbacks_.onRename(id, nameEntry->get_text());
            }
        });

        colorButton->signal_color_set().connect([this, id, colorButton]
        {
            if (callbacks_.onRecolor)
            {
                callbacks_.onRecolor(id, rgbaToHex(colorButton->get_rgba()));
            }
        });

        removeButton->signal_clicked().connect([this, id, row, nameEntry]
        {
            const std::string categoryName = nameEntry->get_text();
            const auto &loc = LocaleManager::instance();

            new ConfirmDialog(
                *this,
                loc.translate("confirm.deleteCategory.title"),
                loc.translate("confirm.deleteCategory.prefix") + " \"" + categoryName +
                "\"? " + loc.translate("confirm.deleteCategory.suffix"),
                loc.translate("action.delete"),
                [this, id, row]
                {
                    if (callbacks_.onRemove)
                    {
                        callbacks_.onRemove(id);
                    }

                    listBox_.remove(*row);
                });
        });
    }
}
