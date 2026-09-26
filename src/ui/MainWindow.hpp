#pragma once

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/label.h>

#include "core/GroceriesDocument.hpp"
#include "core/MonthCoordinator.hpp"
#include "ui/GroceriesMenu.hpp"
#include "ui/MoneyFormatter.hpp"
#include "ui/MonthSummaryPanel.hpp"
#include "ui/ProductTableView.hpp"

#include "stapik/cloud/CloudStorageClient.hpp"

#include <memory>

namespace groceries::ui
{
    class MainWindow : public Gtk::ApplicationWindow
    {
    public:
        explicit MainWindow();

        ~MainWindow() override;

    private:
        core::GroceriesDocument document_;
        core::MonthCoordinator monthCoordinator_;
        std::unique_ptr<CloudStorageClient> cloudClient_;
        domain::MonthId currentMonth_;

        GroceriesMenu menu_;

        Gtk::HeaderBar headerBar_;
        std::vector<domain::MonthId> monthOrder_;

        Gtk::Box rootBox_{Gtk::Orientation::VERTICAL};
        Gtk::Box actionsBar_{Gtk::Orientation::HORIZONTAL, 8};
        Gtk::Button addProductButton_;
        Gtk::Label currentMonthLabel_;
        MoneyFormatter moneyFormatter_;
        MonthSummaryPanel summaryPanel_;
        ProductTableView tableView_;

        void loadOrInitializeDocument();

        void initCloud();

        void setCloudClient(std::unique_ptr<CloudStorageClient> client);

        void syncFromCloud();

        void applyDocumentCurrency();

        void saveDocument();

        void refreshCurrentMonthView();

        void refreshMonthMenu();

        void retranslateUi();

        void onNewMonthClicked();

        void onCategoriesClicked();

        void onStatsClicked();

        void onAddProductClicked();

        void onSelectMonth(std::size_t index);

        void onDeleteMonthClicked();

        void onSetCurrency(const std::string &code);

        void onConnectClicked();

        void onSyncClicked();

        void onWeeklyPlanClicked();

        void onOffBudgetChanged(double newAmount);

        void onRecordPurchase(const std::string &productId);

        void onEditProduct(const std::string &productId);

        void onDeleteProduct(const std::string &productId);
    };
}
