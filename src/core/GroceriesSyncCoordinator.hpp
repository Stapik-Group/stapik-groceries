#pragma once

#include "core/GroceriesDocument.hpp"
#include "stapik/cloud/CloudStorageClient.hpp"

namespace groceries::core
{
    class GroceriesSyncCoordinator
    {
    public:
        [[nodiscard]] static GroceriesDocument resolveOnConnect(const GroceriesDocument &local, const CloudStorageClient &cloudClient);
        [[nodiscard]] static GroceriesDocument pushLocalChange(const GroceriesDocument &local, const CloudStorageClient &cloudClient);
    private:
        [[nodiscard]] static GroceriesDocument pushWithConflictResolution(
            const GroceriesDocument &local, const CloudStorageClient &cloudClient,
            std::optional<std::chrono::system_clock::time_point> baseline);
        [[nodiscard]] static GroceriesDocument fromCloudDocument(const CloudDocument &document);
    };
}
