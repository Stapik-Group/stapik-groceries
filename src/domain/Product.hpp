#pragma once

#include <string>

namespace groceries::domain
{
    class Product
    {
    public:
        Product() = default;

        Product(std::string id,
                std::string name,
                std::string categoryId,
                std::string unitName,
                double plannedQuantity,
                double plannedUnitPrice);

        void rename(std::string newName);
        void recategorize(std::string newCategoryId);
        void setUnitName(std::string newUnitName);
        void setPlan(double quantity, double unitPrice);
        void recordPurchase(double quantity, double totalPaid);
        void resetPurchasesForNewMonth();
        void restorePurchaseState(double purchasedQuantity, double realAverageUnitPrice);

        [[nodiscard]] const std::string &id() const { return id_; }
        [[nodiscard]] const std::string &name() const { return name_; }
        [[nodiscard]] const std::string &categoryId() const { return categoryId_; }
        [[nodiscard]] const std::string &unitName() const { return unitName_; }

        [[nodiscard]] double plannedQuantity() const { return plannedQuantity_; }
        [[nodiscard]] double plannedUnitPrice() const { return plannedUnitPrice_; }
        [[nodiscard]] double purchasedQuantity() const { return purchasedQuantity_; }
        [[nodiscard]] double realAverageUnitPrice() const { return realAverageUnitPrice_; }

        [[nodiscard]] double plannedTotal() const;
        [[nodiscard]] double realTotal() const;
        [[nodiscard]] double savings() const;
        [[nodiscard]] bool metPlannedQuantity() const;

    private:
        std::string id_;
        std::string name_;
        std::string categoryId_;
        std::string unitName_;
        double plannedQuantity_ = 0.0;
        double plannedUnitPrice_ = 0.0;
        double purchasedQuantity_ = 0.0;
        double realAverageUnitPrice_ = 0.0;
    };
}
