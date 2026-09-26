#include "ui/WeeklyPlanExporter.hpp"

#include <cstdio>
#include <fstream>
#include <map>
#include <sstream>

#include "stapik/locale/LocaleManager.hpp"

namespace groceries::ui
{
    using core::WeeklyPlanGenerator;
    using core::WeeklyPlanItem;

    namespace
    {
        std::string htmlEscape(const std::string &text)
        {
            std::string escaped;
            escaped.reserve(text.size());
            for (const char c: text)
            {
                switch (c)
                {
                    case '&': escaped += "&amp;";
                        break;
                    case '<': escaped += "&lt;";
                        break;
                    case '>': escaped += "&gt;";
                        break;
                    case '"': escaped += "&quot;";
                        break;
                    default: escaped += c;
                }
            }
            return escaped;
        }

        std::string formatQuantity(double value, bool isWholeUnits)
        {
            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), isWholeUnits ? "%.0f" : "%.2g", value);
            return std::string(buffer);
        }
    }

    std::filesystem::path WeeklyPlanExporter::exportToHtml(const WeeklyPlanGenerator::Result &plan,
                                                           const MoneyFormatter &formatter,
                                                           const std::filesystem::path &outputPath)
    {
        const auto &loc = LocaleManager::instance();

        std::vector<std::string> categoryOrder;
        std::map<std::string, std::vector<const WeeklyPlanItem *> > byCategory;
        for (const WeeklyPlanItem &item: plan.items)
        {
            if (byCategory.find(item.categoryName) == byCategory.end())
            {
                categoryOrder.push_back(item.categoryName);
            }
            byCategory[item.categoryName].push_back(&item);
        }

        std::ostringstream html;
        html << "<!DOCTYPE html>\n<html lang=\"pl\">\n<head>\n<meta charset=\"utf-8\">\n";
        html << "<title>" << htmlEscape(loc.translate("weeklyPlan.title")) << "</title>\n";
        html << R"CSS(<style>
  :root { color-scheme: light; }
  body { font-family: system-ui, sans-serif; max-width: 640px; margin: 24px auto; padding: 0 16px;
         color: #1a1a1a; }
  h1 { font-size: 1.4em; margin-bottom: 0; }
  .subtitle { color: #555; margin-top: 4px; margin-bottom: 24px; }
  .print-hint { font-size: 0.9em; color: #777; margin-bottom: 24px; }
  h2 { font-size: 1.05em; margin-top: 28px; margin-bottom: 6px; border-bottom: 1px solid #ddd;
       padding-bottom: 4px; }
  ul { list-style: none; padding: 0; margin: 0; }
  li { padding: 6px 2px; border-bottom: 1px solid #eee; display: flex; align-items: baseline; gap: 8px; }
  li input[type="checkbox"] { width: 18px; height: 18px; flex-shrink: 0; }
  .qty { color: #555; }
  .empty { color: #777; font-style: italic; }
  @media print {
    .print-hint { display: none; }
    body { max-width: none; margin: 0; padding: 0 8px; }
    li input[type="checkbox"] { -webkit-print-color-adjust: exact; }
  }
</style>
)CSS";
        html << "</head>\n<body>\n";

        html << "<h1>" << htmlEscape(loc.translate("weeklyPlan.title")) << "</h1>\n";
        html << "<div class=\"subtitle\">" << htmlEscape(loc.translate("weeklyPlan.week.prefix")) << " "
                << plan.currentWeekNumber << " " << htmlEscape(loc.translate("weeklyPlan.week.of")) << " "
                << plan.totalWeeksInMonth << "</div>\n";
        html << "<div class=\"print-hint\">" << htmlEscape(loc.translate("weeklyPlan.printHint")) << "</div>\n";

        if (plan.items.empty())
        {
            html << "<p class=\"empty\">" << htmlEscape(loc.translate("weeklyPlan.empty.body")) << "</p>\n";
        } else
        {
            for (const std::string &categoryName: categoryOrder)
            {
                const std::string heading =
                        categoryName.empty() ? loc.translate("weeklyPlan.noCategory") : categoryName;
                html << "<h2>" << htmlEscape(heading) << "</h2>\n<ul>\n";
                for (const WeeklyPlanItem *item: byCategory[categoryName])
                {
                    html << "<li><label style=\"display:flex; align-items:baseline; gap:8px; width:100%;\">"
                            << "<input type=\"checkbox\"><span>" << htmlEscape(item->productName)
                            << "</span><span class=\"qty\">&mdash; " << formatQuantity(
                                item->quantity, item->isWholeUnits)
                            << " " << htmlEscape(item->unitName) << " &middot; "
                            << htmlEscape(formatter.format(item->plannedUnitPrice)) << " / "
                            << htmlEscape(item->unitName) << "</span></label></li>\n";
                }
                html << "</ul>\n";
            }
        }

        html << "</body>\n</html>\n";

        std::ofstream file(outputPath, std::ios::binary | std::ios::trunc);
        file << html.str();
        file.close();

        return outputPath;
    }
}
