#include "ui/MainWindow.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <random>
#include <tuple>

#include "AppInfo.hpp"
#include "domain/Currency.hpp"
#include "core/GroceriesSyncCoordinator.hpp"
#include "persistence/JsonStore.hpp"
#include "stapik/cloud/CloudStorageException.hpp"
#include "stapik/storage/CloudStorageConfigStorage.hpp"
#include "stapik/ui/dialog/ConnectDialog.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "ui/CategoryManagerDialog.hpp"
#include "ui/ConfirmDialog.hpp"
#include "ui/ProductDialog.hpp"
#include "ui/PurchaseDialog.hpp"
#include "ui/StatsWindow.hpp"
#include "ui/WeeklyPlanExporter.hpp"

#include "core/WeeklyPlanGenerator.hpp"
#include "stapik/storage/AppPaths.hpp"

#include <giomm/appinfo.h>
#include <giomm/file.h>

namespace groceries::ui
{
    using core::GroceriesDocument;
    using core::MonthCoordinator;
    using domain::Category;
    using domain::CurrencyCatalog;
    using domain::MonthId;
    using domain::MonthlyList;
    using domain::Product;
    using core::GroceriesSyncCoordinator;
    using persistence::JsonStore;

    namespace
    {
        MonthId currentCalendarMonth()
        {
            const std::time_t now = std::time(nullptr);
            std::tm localTime{};
            localtime_r(&now, &localTime);

            MonthId id;
            id.year = localTime.tm_year + 1900;
            id.month = localTime.tm_mon + 1;
            return id;
        }

        std::int64_t nowEpochMs()
        {
            const auto now = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        }

        std::string generateId(const std::string &prefix)
        {
            static std::mt19937_64 rng(std::random_device{}());
            std::uniform_int_distribution<int> dist(0, 0xFFFF);
            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "%s-%llx-%04x", prefix.c_str(),
                          static_cast<unsigned long long>(nowEpochMs()), dist(rng));
            return std::string(buffer);
        }

        const char *const kMonthKeys[] = {
            "month.january", "month.february", "month.march", "month.april",
            "month.may", "month.june", "month.july", "month.august",
            "month.september", "month.october", "month.november", "month.december"
        };

        std::string formatMonthLabel(const MonthId &id)
        {
            const std::string name = (id.month >= 1 && id.month <= 12)
                                         ? LocaleManager::instance().translate(kMonthKeys[id.month - 1])
                                         : "?";
            return name + " " + std::to_string(id.year);
        }
    } // namespace

    MainWindow::MainWindow()
        : monthCoordinator_(document_),
          menu_(
              *this,
              GroceriesMenuCallbacks{
                  .onNewMonth = sigc::mem_fun(*this, &MainWindow::onNewMonthClicked),
                  .onCategories = sigc::mem_fun(*this, &MainWindow::onCategoriesClicked),
                  .onStats = sigc::mem_fun(*this, &MainWindow::onStatsClicked),
                  .onSelectMonth = sigc::mem_fun(*this, &MainWindow::onSelectMonth),
                  .onDeleteMonth = sigc::mem_fun(*this, &MainWindow::onDeleteMonthClicked),
                  .onSetCurrency = sigc::mem_fun(*this, &MainWindow::onSetCurrency),
                  .onConnect = sigc::mem_fun(*this, &MainWindow::onConnectClicked),
                  .onSync = sigc::mem_fun(*this, &MainWindow::onSyncClicked),
                  .onWeeklyPlan = sigc::mem_fun(*this, &MainWindow::onWeeklyPlanClicked)
              }),
          summaryPanel_(moneyFormatter_),
          tableView_(moneyFormatter_)
    {
        set_title("Stapik Groceries");
        set_default_size(1000, 650);
        set_titlebar(headerBar_);

        retranslateUi();
        LocaleManager::instance().signalLocaleChanged().connect([this]
        {
            retranslateUi();
            currentMonthLabel_.set_text(formatMonthLabel(currentMonth_));
            refreshMonthMenu();
        });

        actionsBar_.set_margin(8);
        actionsBar_.add_css_class("groceries-actionsbar");
        actionsBar_.append(addProductButton_);
        currentMonthLabel_.add_css_class("groceries-current-month");
        currentMonthLabel_.set_hexpand(true);
        currentMonthLabel_.set_halign(Gtk::Align::END);
        actionsBar_.append(currentMonthLabel_);
        addProductButton_.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onAddProductClicked));

        rootBox_.append(menu_.getMenuBar());
        rootBox_.append(actionsBar_);
        rootBox_.append(summaryPanel_);
        rootBox_.append(tableView_);
        rootBox_.add_css_class("groceries-window-frame");
        set_child(rootBox_);

        summaryPanel_.onOffBudgetChanged = sigc::mem_fun(*this, &MainWindow::onOffBudgetChanged);
        tableView_.onRecordPurchase = sigc::mem_fun(*this, &MainWindow::onRecordPurchase);
        tableView_.onEditProduct = sigc::mem_fun(*this, &MainWindow::onEditProduct);
        tableView_.onDeleteProduct = sigc::mem_fun(*this, &MainWindow::onDeleteProduct);

        loadOrInitializeDocument();
        applyDocumentCurrency();
        refreshMonthMenu();
        refreshCurrentMonthView();
        initCloud();
    }

    MainWindow::~MainWindow() = default;

    void MainWindow::retranslateUi()
    {
        const auto &loc = LocaleManager::instance();
        addProductButton_.set_label(loc.translate("actions.addProduct"));
    }

    void MainWindow::loadOrInitializeDocument()
    {
        try
        {
            document_ = JsonStore::loadFromFile(JsonStore::defaultDataFilePath());
        } catch (const std::exception &e)
        {
            std::cerr << "Nie udało się wczytać danych (" << e.what() << "), startuję od pustego dokumentu.\n";
            document_ = GroceriesDocument{};
        }

        if (auto latest = document_.latestMonth())
        {
            currentMonth_ = *latest;
        } else
        {
            currentMonth_ = currentCalendarMonth();
            monthCoordinator_.startMonth(currentMonth_);
            saveDocument();
        }
    }

    void MainWindow::initCloud()
    {
        const auto config = CloudStorageConfigStorage::load(APP_NAME);
        if (!config.has_value())
        {
            return;
        }
        setCloudClient(std::make_unique<CloudStorageClient>(config.value(), GROCERIES_FILENAME));
    }

    void MainWindow::setCloudClient(std::unique_ptr<CloudStorageClient> client)
    {
        cloudClient_ = std::move(client);
        syncFromCloud();
    }

    void MainWindow::syncFromCloud()
    {
        if (cloudClient_ == nullptr)
        {
            return;
        }

        document_ = GroceriesSyncCoordinator::resolveOnConnect(document_, *cloudClient_);
        applyDocumentCurrency();

        try
        {
            JsonStore::saveToFile(document_, JsonStore::defaultDataFilePath());
        } catch (const std::exception &e)
        {
            std::cerr << "Nie udało się zapisać danych: " << e.what() << "\n";
        }

        if (auto latest = document_.latestMonth())
        {
            currentMonth_ = *latest;
        }
        refreshMonthMenu();
        refreshCurrentMonthView();
    }

    void MainWindow::onConnectClicked()
    {
        auto *dialog = new ConnectDialog(*this);

        if (const auto config = CloudStorageConfigStorage::load(APP_NAME);
            config.has_value())
        {
            dialog->prefillConfig(config.value());
        }

        dialog->signal_response().connect([this, dialog](int responseId)
        {
            if (responseId == Gtk::ResponseType::OK)
            {
                if (const auto result = dialog->getResult(); result.has_value())
                {
                    const auto &loc = LocaleManager::instance();
                    CloudStorageConfigStorage::save(result.value(), APP_NAME);
                    try
                    {
                        auto client = std::make_unique<CloudStorageClient>(result.value(), GROCERIES_FILENAME);
                        std::ignore = client->loadDocument();
                        setCloudClient(std::move(client));
                        showMessageDialog(*this, loc.translate("cloud.connected"),
                                          loc.translate("cloud.connected.secondary"), Gtk::MessageType::INFO);
                    } catch (const CloudStorageException &e)
                    {
                        showMessageDialog(*this, loc.translate("cloud.failed.header"), e.what(),
                                          Gtk::MessageType::ERROR);
                    }
                }
            }
            dialog->hide();
        });
        dialog->signal_hide().connect([dialog] { delete dialog; });
        dialog->show();
    }

    void MainWindow::onSyncClicked()
    {
        syncFromCloud();
    }

    void MainWindow::applyDocumentCurrency()
    {
        const auto &currency = CurrencyCatalog::findByCode(document_.currencyCode());
        moneyFormatter_.setCurrency(currency);
        menu_.setCurrency(currency.code);
    }

    void MainWindow::onSetCurrency(const std::string &code)
    {
        document_.setCurrencyCode(CurrencyCatalog::findByCode(code).code);
        applyDocumentCurrency();
        refreshCurrentMonthView();
        saveDocument();
    }

    void MainWindow::saveDocument()
    {
        document_.touch(nowEpochMs());

        if (cloudClient_ != nullptr)
        {
            document_ = GroceriesSyncCoordinator::pushLocalChange(document_, *cloudClient_);
        }

        try
        {
            JsonStore::saveToFile(document_, JsonStore::defaultDataFilePath());
        } catch (const std::exception &e)
        {
            std::cerr << "Failed to save data: " << e.what() << "\n";
        }
    }

    void MainWindow::refreshMonthMenu()
    {
        monthOrder_.clear();
        std::vector<Glib::ustring> labels;
        std::size_t selectedIndex = 0;
        for (const MonthlyList &month: document_.months())
        {
            if (month.month() == currentMonth_)
            {
                selectedIndex = monthOrder_.size();
            }
            monthOrder_.push_back(month.month());
            labels.push_back(formatMonthLabel(month.month()));
        }
        menu_.setMonths(labels, selectedIndex);
    }

    void MainWindow::refreshCurrentMonthView()
    {
        currentMonthLabel_.set_text(formatMonthLabel(currentMonth_));

        MonthlyList *month = document_.findMonth(currentMonth_);
        if (month == nullptr)
        {
            month = &monthCoordinator_.startMonth(currentMonth_);
        }

        summaryPanel_.refresh(*month);
        tableView_.refresh(*month, document_.categories());
    }

    void MainWindow::onNewMonthClicked()
    {
        MonthlyList &next = monthCoordinator_.startNextMonth();
        currentMonth_ = next.month();
        refreshMonthMenu();
        refreshCurrentMonthView();
        saveDocument();
    }

    void MainWindow::onSelectMonth(std::size_t index)
    {
        if (index >= monthOrder_.size())
        {
            return;
        }
        currentMonth_ = monthOrder_[index];
        refreshCurrentMonthView();
    }

    void MainWindow::onDeleteMonthClicked()
    {
        const MonthId target = currentMonth_;
        const auto &loc = LocaleManager::instance();

        new ConfirmDialog(*this, loc.translate("confirm.deleteMonth.title"),
                          loc.translate("confirm.deleteMonth.prefix") + " " + formatMonthLabel(target) +
                          "? " + loc.translate("confirm.deleteMonth.suffix"),
                          loc.translate("action.delete"),
                          [this, target]()
                          {
                              if (!document_.removeMonth(target))
                              {
                                  return;
                              }
                              if (auto latest = document_.latestMonth())
                              {
                                  currentMonth_ = *latest;
                              } else
                              {
                                  currentMonth_ = currentCalendarMonth();
                                  monthCoordinator_.startMonth(currentMonth_);
                              }
                              refreshMonthMenu();
                              refreshCurrentMonthView();
                              saveDocument();
                          });
    }

    void MainWindow::onCategoriesClicked()
    {
        CategoryManagerCallbacks callbacks;

        callbacks.onAdd = [this](const std::string &name) -> Category
        {
            Category created(generateId("cat"), name, "#CCCCCC");
            document_.addCategory(created);
            saveDocument();
            return created;
        };
        callbacks.onRename = [this](const std::string &id, const std::string &newName)
        {
            if (Category *category = document_.findCategory(id))
            {
                category->rename(newName);
                saveDocument();
            }
        };
        callbacks.onRecolor = [this](const std::string &id, const std::string &newColorHex)
        {
            if (Category *category = document_.findCategory(id))
            {
                category->recolor(newColorHex);
                saveDocument();
            }
        };
        callbacks.onRemove = [this](const std::string &id)
        {
            document_.removeCategory(id);
            saveDocument();
            refreshCurrentMonthView();
        };

        new CategoryManagerDialog(*this, document_.categories(), std::move(callbacks));
    }

    void MainWindow::onWeeklyPlanClicked()
    {
        const auto &loc = LocaleManager::instance();

        const MonthlyList *month = document_.findMonth(currentMonth_);
        if (month == nullptr)
        {
            return;
        }

        std::tm localTime{};
        if (currentMonth_ == currentCalendarMonth())
        {
            const std::time_t now = std::time(nullptr);
            localtime_r(&now, &localTime);
        } else
        {
            localTime.tm_year = currentMonth_.year - 1900;
            localTime.tm_mon = currentMonth_.month - 1;
            localTime.tm_mday = 1;
            localTime.tm_hour = 12;
            std::mktime(&localTime);
        }

        const auto plan = core::WeeklyPlanGenerator::generate(*month, document_.categories(), localTime);
        if (plan.items.empty())
        {
            showMessageDialog(*this, loc.translate("weeklyPlan.empty.title"),
                              loc.translate("weeklyPlan.empty.body"), Gtk::MessageType::INFO);
            return;
        }

        char dateSuffix[32];
        std::snprintf(dateSuffix, sizeof(dateSuffix), "%04d-%02d-%02d", localTime.tm_year + 1900,
                      localTime.tm_mon + 1, localTime.tm_mday);
        const auto outputPath =
                AppPaths::userDataDir(APP_NAME) / (std::string("weekly-list-") + dateSuffix + ".html");

        WeeklyPlanExporter::exportToHtml(plan, moneyFormatter_, outputPath);

        try
        {
            Gio::AppInfo::launch_default_for_uri(Gio::File::create_for_path(outputPath.string())->get_uri());
        } catch (const Glib::Error &)
        {
            showMessageDialog(*this, loc.translate("weeklyPlan.title"), outputPath.string(),
                              Gtk::MessageType::INFO);
        }
    }

    void MainWindow::onStatsClicked()
    {
        new StatsWindow(*this, document_.months(), moneyFormatter_);
    }

    void MainWindow::onAddProductClicked()
    {
        MonthlyList *month = document_.findMonth(currentMonth_);
        if (month == nullptr)
        {
            month = &monthCoordinator_.startMonth(currentMonth_);
        }
        std::vector<std::string> existingNames;
        for (const Product &p: month->products())
        {
            existingNames.push_back(p.name());
        }

        new ProductDialog(*this, document_.categories(), nullptr, std::move(existingNames),
                          [this](const ProductFormValues &values)
                          {
                              MonthlyList *m = document_.findMonth(currentMonth_);
                              if (m == nullptr)
                              {
                                  m = &monthCoordinator_.startMonth(currentMonth_);
                              }
                              m->addProduct(Product(generateId("prod"), values.name, values.categoryId,
                                                    values.unitName, values.plannedQuantity,
                                                    values.plannedUnitPrice));
                              refreshCurrentMonthView();
                              saveDocument();
                          });
    }

    void MainWindow::onEditProduct(const std::string &productId)
    {
        MonthlyList *month = document_.findMonth(currentMonth_);
        if (month == nullptr)
        {
            return;
        }
        Product *product = month->findProduct(productId);
        if (product == nullptr)
        {
            return;
        }

        std::vector<std::string> existingNames;
        for (const Product &p: month->products())
        {
            if (p.id() != productId)
            {
                existingNames.push_back(p.name());
            }
        }

        new ProductDialog(*this, document_.categories(), product, std::move(existingNames),
                          [this, productId](const ProductFormValues &values)
                          {
                              MonthlyList *m = document_.findMonth(currentMonth_);
                              Product *p = m != nullptr ? m->findProduct(productId) : nullptr;
                              if (p == nullptr)
                              {
                                  return;
                              }
                              p->rename(values.name);
                              p->recategorize(values.categoryId);
                              p->setUnitName(values.unitName);
                              p->setPlan(values.plannedQuantity, values.plannedUnitPrice);
                              refreshCurrentMonthView();
                              saveDocument();
                          });
    }

    void MainWindow::onDeleteProduct(const std::string &productId)
    {
        MonthlyList *month = document_.findMonth(currentMonth_);
        if (month == nullptr)
        {
            return;
        }
        Product *product = month->findProduct(productId);
        if (product == nullptr)
        {
            return;
        }

        const auto &loc = LocaleManager::instance();
        new ConfirmDialog(*this, loc.translate("confirm.deleteProduct.title"),
                          loc.translate("confirm.deleteProduct.prefix") + " \"" + product->name() + "\" " +
                          loc.translate("confirm.deleteProduct.suffix"),
                          loc.translate("action.delete"),
                          [this, productId]()
                          {
                              MonthlyList *m = document_.findMonth(currentMonth_);
                              if (m == nullptr)
                              {
                                  return;
                              }
                              m->removeProduct(productId);
                              refreshCurrentMonthView();
                              saveDocument();
                          });
    }

    void MainWindow::onRecordPurchase(const std::string &productId)
    {
        MonthlyList *month = document_.findMonth(currentMonth_);
        if (month == nullptr)
        {
            return;
        }
        Product *product = month->findProduct(productId);
        if (product == nullptr)
        {
            return;
        }

        new PurchaseDialog(*this, product->name(), product->unitName(), moneyFormatter_,
                           [this, productId](double quantity, double totalPaid)
                           {
                               MonthlyList *m = document_.findMonth(currentMonth_);
                               Product *p = m != nullptr ? m->findProduct(productId) : nullptr;
                               if (p == nullptr)
                               {
                                   return;
                               }
                               p->recordPurchase(quantity, totalPaid);
                               refreshCurrentMonthView();
                               saveDocument();
                           });
    }

    void MainWindow::onOffBudgetChanged(double newAmount)
    {
        MonthlyList *month = document_.findMonth(currentMonth_);
        if (month == nullptr)
        {
            return;
        }
        month->setOffBudgetAmount(newAmount);
        refreshCurrentMonthView();
        saveDocument();
    }
}
