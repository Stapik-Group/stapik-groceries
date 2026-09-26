#include "core/GroceriesSyncCoordinator.hpp"

#include "persistence/JsonStore.hpp"
#include "stapik/cloud/CloudStorageException.hpp"

#include <glib.h>

namespace groceries::core {

using persistence::JsonStore;

namespace {

std::int64_t epochMsFromTimePoint(std::chrono::system_clock::time_point tp) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

std::chrono::system_clock::time_point timePointFromEpochMs(std::int64_t epochMs) {
    return std::chrono::system_clock::time_point{std::chrono::milliseconds(epochMs)};
}

} // namespace

GroceriesDocument GroceriesSyncCoordinator::fromCloudDocument(const CloudDocument& document) {
    GroceriesDocument doc = JsonStore::fromJson(document.content);
    doc.setLastKnownCloudUpdateEpochMs(epochMsFromTimePoint(document.updatedAt));
    return doc;
}

GroceriesDocument GroceriesSyncCoordinator::pushWithConflictResolution(
    const GroceriesDocument& local, const CloudStorageClient& cloudClient,
    const std::optional<std::chrono::system_clock::time_point> baseline) {
    CloudWriteResult result;

    try {
        result = cloudClient.saveDocument(JsonStore::toJson(local),
                                           baseline.value_or(std::chrono::system_clock::time_point{}));
    } catch (const CloudStorageException& e) {
        // TYMCZASOWE (diagnostyka): g_warning zamiast g_debug, żeby ten błąd było
        // w ogóle widać — GLib domyślnie wycisza g_debug w konsoli, więc realny
        // powód nieudanego zapisu (status HTTP / błąd curla) ginął bez śladu.
        g_warning("[Cloud] Zapis do chmury nie powiódł się: %s", e.what());
        return local;
    }

    if (!result.conflict) {
        GroceriesDocument doc = local;
        doc.setLastKnownCloudUpdateEpochMs(epochMsFromTimePoint(result.document.updatedAt));
        return doc;
    }

    // Serwer ma nowszy dokument, niż nam było wiadomo — wygrywa.
    if (epochMsFromTimePoint(result.document.updatedAt) > local.lastUpdateEpochMs()) {
        return fromCloudDocument(result.document);
    }

    // Wciąż jesteśmy nowsi (rzadki wyścig) — jedna próba wobec aktualnego baseline'u serwera.
    try {
        const auto [document, conflict] =
            cloudClient.saveDocument(JsonStore::toJson(local), result.document.updatedAt);

        if (!conflict) {
            GroceriesDocument doc = local;
            doc.setLastKnownCloudUpdateEpochMs(epochMsFromTimePoint(document.updatedAt));
            return doc;
        }

        // Przegraliśmy wyścig drugi raz — akceptujemy wersję serwera, żeby nie zapętlać.
        return fromCloudDocument(document);
    } catch (const CloudStorageException& e) {
        g_warning("[Cloud] Zapis do chmury nie powiódł się (retry): %s", e.what());
        return local;
    }
}

GroceriesDocument GroceriesSyncCoordinator::resolveOnConnect(const GroceriesDocument& local,
                                                               const CloudStorageClient& cloudClient) {
    std::optional<CloudDocument> remote;

    try {
        remote = cloudClient.loadDocument();
    } catch (const CloudStorageException& e) {
        g_warning("[Cloud] Odczyt z chmury nie powiódł się: %s", e.what());
        return local;

    }

    // Ta konkretna instancja Stapik Cloud zwraca 200 z pustym `content` dla
    // slotu, który istnieje (jest zarejestrowany), ale nigdy nie miał
    // zapisanego dokumentu — a nie 404 (patrz CloudStorageClient::parseDocumentResponse
    // w stapik-common, które jawnie obsługuje pusty content jako poprawną odpowiedź).
    // Taki pusty stub traktujemy tak samo jak brak dokumentu: mamy co pchnąć,
    // więc pchamy lokalne dane zamiast łykać pustkę jako "nowszą wersję".
    const bool remoteIsEmptyStub = remote.has_value() && remote->content.is_object() && remote->content.empty();

    if (!remote.has_value() || remoteIsEmptyStub) {
        return pushWithConflictResolution(local, cloudClient, std::nullopt);
    }

    if (epochMsFromTimePoint(remote->updatedAt) > local.lastUpdateEpochMs()) {
        return fromCloudDocument(remote.value());
    }

    return pushWithConflictResolution(local, cloudClient, remote->updatedAt);
}

GroceriesDocument GroceriesSyncCoordinator::pushLocalChange(const GroceriesDocument& local,
                                                              const CloudStorageClient& cloudClient) {
    std::optional<std::chrono::system_clock::time_point> baseline;
    if (local.lastKnownCloudUpdateEpochMs().has_value()) {
        baseline = timePointFromEpochMs(*local.lastKnownCloudUpdateEpochMs());
    }
    return pushWithConflictResolution(local, cloudClient, baseline);
}

} // namespace groceries::core
