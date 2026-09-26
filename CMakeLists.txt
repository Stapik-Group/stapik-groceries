cmake_minimum_required(VERSION 4.2)
project(stapikgroceries VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_NAME_DISPLAY "Stapik Groceries")
set(PROJECT_AUTHOR "Sebastian Smoliński")
set(PROJECT_GITHUB_URL "https://github.com/Stapik-Group/stapik-groceries")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

find_package(CURL REQUIRED)

include(FetchContent)
FetchContent_Declare(
        stapikcommon
        GIT_REPOSITORY https://github.com/Stapik-Group/stapik-common.git
        GIT_TAG v1.1.6
)
FetchContent_MakeAvailable(stapikcommon)

add_executable(stapikgroceries
        src/main.cpp
        src/domain/Category.cpp
        src/domain/Currency.cpp
        src/domain/Product.cpp
        src/domain/MonthlyList.cpp
        src/persistence/JsonStore.cpp
        src/core/GroceriesDocument.cpp
        src/core/MonthCoordinator.cpp
        src/core/GroceriesSyncCoordinator.cpp
        src/core/WeeklyPlanGenerator.cpp
        src/ui/GroceriesMenu.cpp
        src/ui/MainWindow.cpp
        src/ui/MoneyFormatter.cpp
        src/ui/MonthSummaryPanel.cpp
        src/ui/ProductTableView.cpp
        src/ui/StatsChartView.cpp
        src/ui/ProductDialog.cpp
        src/ui/PurchaseDialog.cpp
        src/ui/CategoryManagerDialog.cpp
        src/ui/ConfirmDialog.cpp
        src/ui/StatsWindow.cpp
        src/ui/WeeklyPlanExporter.cpp)

target_include_directories(stapikgroceries PRIVATE src ${GTKMM_INCLUDE_DIRS})
target_compile_options(stapikgroceries PRIVATE ${GTKMM_CFLAGS_OTHER})
target_link_libraries(stapikgroceries PRIVATE ${GTKMM_LIBRARIES} stapikcommon CURL::libcurl nlohmann_json::nlohmann_json)

add_custom_command(TARGET stapikgroceries POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:stapikgroceries>/resources)

install(TARGETS stapikgroceries
        RUNTIME DESTINATION lib/stapikgroceries
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/
        DESTINATION lib/stapikgroceries/resources
)

install(PROGRAMS ${CMAKE_SOURCE_DIR}/packaging/stapikgroceries-launcher
        DESTINATION bin
        RENAME stapikgroceries
)

install(FILES ${CMAKE_SOURCE_DIR}/packaging/stapikgroceries.desktop
        DESTINATION share/applications
)

install(FILES ${CMAKE_SOURCE_DIR}/resources/icon_256.png
        DESTINATION share/icons/hicolor/256x256/apps
        RENAME stapikgroceries.png
)

set(CPACK_PACKAGE_NAME "stapikgroceries")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR ${PROJECT_AUTHOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A retro-styled desktop grocery-budgeting application")
set(CPACK_PACKAGE_CONTACT "sebastian.smolinski6@gmail.com")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${PROJECT_AUTHOR} <sebastian.smolinski6@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

include(CPack)