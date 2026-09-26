#include "domain/Product.hpp"

#include <utility>

namespace groceries::domain
{
    Product::Product(std::string id,
                     std::string name,
                     std::string categoryId,
                     std::string unitName,
                     double plannedQuantity,
                     double plannedUnitPrice) :
        id_(std::move(id)),
        name_(std::move(name)),
        categoryId_(std::move(categoryId)),
        unitName_(std::move(unitName)),
        plannedQuantity_(plannedQuantity),
        plannedUnitPrice_(plannedUnitPrice) { }

    void Product::rename(std::string newName)
    {
        name_ = std::move(newName);
    }

    void Product::recategorize(std::string newCategoryId)
    {
        categoryId_ = std::move(newCategoryId);
    }

    void Product::setUnitName(std::string newUnitName)
    {
        unitName_ = std::move(newUnitName);
    }

    void Product::setPlan(const double quantity, const double unitPrice)
    {
        plannedQuantity_ = quantity;
        plannedUnitPrice_ = unitPrice;
    }

    void Product::recordPurchase(const double quantity, const double totalPaid)
    {
        if (quantity <= 0.0)
        {
            return;
        }

        const double previousTotalPaid = realAverageUnitPrice_ * purchasedQuantity_;
        purchasedQuantity_ += quantity;
        realAverageUnitPrice_ = (previousTotalPaid + totalPaid) / purchasedQuantity_;
    }

    void Product::resetPurchasesForNewMonth()
    {
        purchasedQuantity_ = 0.0;
        realAverageUnitPrice_ = 0.0;
    }

    void Product::restorePurchaseState(const double purchasedQuantity, const double realAverageUnitPrice)
    {
        purchasedQuantity_ = purchasedQuantity;
        realAverageUnitPrice_ = realAverageUnitPrice;
    }

    double Product::plannedTotal() const
    {
        return plannedQuantity_ * plannedUnitPrice_;
    }

    double Product::realTotal() const
    {
        return realAverageUnitPrice_ * purchasedQuantity_;
    }

    bool Product::metPlannedQuantity() const
    {
        return purchasedQuantity_ >= plannedQuantity_;
    }

    double Product::savings() const
    {
        if (!metPlannedQuantity())
        {
            return 0.0;
        }
        return plannedTotal() - realTotal();
    }
}