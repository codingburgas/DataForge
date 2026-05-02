#include "ui/help_panel.h"
#include "ui/theme.h"
#include "ui/toast.h"
#include "imgui.h"

namespace ui {

    namespace {

        struct FoundFile {
            std::string path;
            std::string name;
            std::uintmax_t size;
            std::time_t    mtime;
        };

        struct ScanState {
            bool                   ran        = false;
            bool                   docsExist  = true;
            std::string            docsPath;
            std::vector<FoundFile> results;
        };

        static ScanState& scanState() {
            static ScanState s;
            return s;
        }

        std::filesystem::path documentsPath() {
            char* userProfile = nullptr;
            std::size_t len = 0;
            if (_dupenv_s(&userProfile, &len, "USERPROFILE") != 0 || userProfile == nullptr) {
                return std::filesystem::path();
            }
            std::filesystem::path docs = std::filesystem::path(userProfile) / "Documents";
            std::free(userProfile);
            return docs;
        }

        std::time_t fileTimeToTimeT(std::filesystem::file_time_type ft) {
            using namespace std::chrono;
            auto sysTime = time_point_cast<system_clock::duration>(
                ft - std::filesystem::file_time_type::clock::now() + system_clock::now());
            return system_clock::to_time_t(sysTime);
        }

        void runScan() {
            ScanState& s = scanState();
            s.results.clear();
            s.ran = true;

            std::filesystem::path docs = documentsPath();
            s.docsPath  = docs.string();
            std::error_code ec;
            if (docs.empty() || !std::filesystem::exists(docs, ec)) {
                s.docsExist = false;
                return;
            }
            s.docsExist = true;

            std::filesystem::recursive_directory_iterator it(
                docs,
                std::filesystem::directory_options::skip_permission_denied,
                ec);
            std::filesystem::recursive_directory_iterator end;
            while (!ec && it != end) {
                std::error_code rec;
                if (it->is_regular_file(rec)) {
                    std::string ext = it->path().extension().string();
                    for (std::size_t k = 0; k < ext.size(); ++k) {
                        char c = ext[k];
                        if (c >= 'A' && c <= 'Z') ext[k] = static_cast<char>(c + ('a' - 'A'));
                    }
                    if (ext == ".dftasks") {
                        FoundFile f{};
                        f.path = it->path().string();
                        f.name = it->path().filename().string();
                        std::error_code se;
                        f.size  = std::filesystem::file_size(it->path(), se);
                        std::error_code te;
                        auto ft = std::filesystem::last_write_time(it->path(), te);
                        f.mtime = te ? 0 : fileTimeToTimeT(ft);
                        s.results.push_back(f);
                    }
                }
                it.increment(ec);
            }

            std::sort(s.results.begin(), s.results.end(),
                [](const FoundFile& a, const FoundFile& b) {
                    return a.mtime > b.mtime;
                });
        }

        std::string formatSize(std::uintmax_t bytes) {
            char buf[32];
            if (bytes < 1024) {
                std::snprintf(buf, sizeof(buf), "%llu B", static_cast<unsigned long long>(bytes));
            } else if (bytes < 1024 * 1024) {
                std::snprintf(buf, sizeof(buf), "%.1f KB", bytes / 1024.0);
            } else {
                std::snprintf(buf, sizeof(buf), "%.1f MB", bytes / (1024.0 * 1024.0));
            }
            return buf;
        }

        std::string formatMtime(std::time_t t) {
            if (t == 0) return "unknown";
            std::tm local{};
            if (localtime_s(&local, &t) != 0) return "unknown";
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
                local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                local.tm_hour, local.tm_min);
            return buf;
        }

        void cardBegin(const char* id, float height) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ColBgCard);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 18.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 18.0f));
            ImGui::BeginChild(id, ImVec2(0.0f, height), true);

            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 mn = ImGui::GetWindowPos();
            ImVec2 mx = ImVec2(mn.x + ImGui::GetWindowWidth(),
                               mn.y + ImGui::GetWindowHeight());
            drawSoftShadow(dl, mn, mx, 18.0f);
            dl->AddRectFilled(mn, mx, cardBgU32(), 18.0f);
            dl->AddRect(mn, mx, cardBorderU32(), 18.0f);
        }

        void cardEnd() {
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();
        }

        void sectionHeading(const char* title, const char* subtitle) {
            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "%s", title);
            ImGui::PopFont();
            if (subtitle != nullptr && subtitle[0] != '\0') {
                ImGui::TextColored(ColTextFaint, "%s", subtitle);
            }
            ImGui::Dummy(ImVec2(1.0f, 6.0f));
        }

        void guideEntry(const char* topic, const char* body) {
            ImGui::Bullet();
            ImGui::SameLine();
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextPrimary, "%s", topic);
            ImGui::PopFont();
            ImGui::PushTextWrapPos(0.0f);
            ImGui::Indent(18.0f);
            ImGui::TextColored(ColTextMuted, "%s", body);
            ImGui::Unindent(18.0f);
            ImGui::PopTextWrapPos();
            ImGui::Dummy(ImVec2(1.0f, 6.0f));
        }

        void renderGuides() {
            cardBegin("##HelpGuides", 0.0f);
            sectionHeading("UI Guide",
                "Walkthrough of every part of the workspace.");

            guideEntry("Sidebar",
                "Switch between Overview, My Tasks, Analytics, Benchmark, and Help. "
                "Click 'Collapse' at the bottom to shrink the sidebar to icon-only.");

            guideEntry("Topbar",
                "Shows the current page title, today's date, and an unsaved/ready "
                "indicator. The '+ New Task' button opens the create dialog.");

            guideEntry("Overview page",
                "Greeting, four metric cards (total, completed, projects, blocked), "
                "the Today's Focus list of unfinished priority tasks, and side cards "
                "for active root projects and recent activity. Click any card to jump "
                "to My Tasks with that task selected.");

            guideEntry("My Tasks page",
                "Three panes: tree on the left for hierarchy navigation, table in the "
                "middle for filtering and sorting, details on the right for editing "
                "the selected task. Use the search box and priority/status filters "
                "above the table to narrow the list.");

            guideEntry("Task tree",
                "Expand/collapse parent tasks. Selecting a node updates the table "
                "and details panes. Right-click a node for create/delete actions.");

            guideEntry("Task table",
                "Click a row to select. Header sort matches the Algorithms menu. "
                "Search box matches title and description.");

            guideEntry("Task details",
                "Read-only view with progress, deadline urgency, descendants count, "
                "and shortcuts to edit or delete. Edit opens the same dialog as 'New "
                "Task'.");

            guideEntry("Analytics page",
                "Charts and counts for status mix, priority mix, and completion "
                "trends across the current store.");

            guideEntry("Benchmark page",
                "Generates a synthetic task list of the chosen size and times "
                "Quick sort vs Bubble sort on it. Use to compare algorithm "
                "behavior on the same data.");

            guideEntry("Status bar",
                "Bottom strip with file path, dirty indicator, and last toast "
                "message.");

            guideEntry("Menu bar",
                "File: New / Open / Save / Save As / Exit. Edit: New Task, edit/"
                "delete selected, Undo. View: switch page or theme. Algorithms: "
                "sort key and algorithm. Help: this page and About.");

            guideEntry("Voice input",
                "The mic icon next to Title and Description fields records speech "
                "and inserts the transcript. Requires Windows speech to be "
                "available; failures show as a toast on launch.");

            guideEntry("Keyboard shortcuts",
                "Ctrl+N new file, Ctrl+O open, Ctrl+S save, Ctrl+Shift+S save as, "
                "Ctrl+Enter new task, Del delete selected, Ctrl+Z undo last "
                "destructive action.");

            cardEnd();
        }

        void renderFinder(UiState& uiState) {
            cardBegin("##HelpFinder", 0.0f);
            sectionHeading("Find lost task files",
                "Searches your Documents folder (recursively) for *.dftasks files.");

            ScanState& s = scanState();

            if (gradientButton("##scanDocs", "Scan Documents", ImVec2(170.0f, 36.0f), 14.0f)) {
                runScan();
                if (!s.docsExist) {
                    pushToast(uiState, "Documents folder not found.");
                } else {
                    char msg[96];
                    std::snprintf(msg, sizeof(msg),
                        "Scan complete: %zu file(s) found.", s.results.size());
                    pushToast(uiState, msg);
                }
            }

            ImGui::SameLine(0, 12.0f);
            if (s.ran) {
                ImGui::TextColored(ColTextFaint, "Searched: %s",
                    s.docsPath.empty() ? "(no Documents folder)" : s.docsPath.c_str());
            } else {
                ImGui::TextColored(ColTextFaint,
                    "Click to search %%USERPROFILE%%\\Documents.");
            }

            ImGui::Dummy(ImVec2(1.0f, 10.0f));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(1.0f, 8.0f));

            if (!s.ran) {
                ImGui::TextColored(ColTextFaint,
                    "Results appear here. Each row has a Load button.");
                cardEnd();
                return;
            }

            if (!s.docsExist) {
                ImGui::TextColored(HEX(0xDC2626),
                    "Documents folder could not be located on this profile.");
                cardEnd();
                return;
            }

            if (s.results.empty()) {
                ImGui::TextColored(ColTextFaint,
                    "No .dftasks files were found under Documents.");
                cardEnd();
                return;
            }

            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextSecondary,
                "%zu file(s) found, sorted by most recently modified.",
                s.results.size());
            ImGui::PopFont();
            ImGui::Dummy(ImVec2(1.0f, 8.0f));

            for (std::size_t i = 0; i < s.results.size(); ++i) {
                const FoundFile& f = s.results[i];

                ImGui::PushID(static_cast<int>(i));
                ImVec2 mn = ImGui::GetCursorScreenPos();
                float rowW = ImGui::GetContentRegionAvail().x;
                ImVec2 mx = ImVec2(mn.x + rowW, mn.y + 70.0f);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(mn, mx, ImGui::ColorConvertFloat4ToU32(ColBgSubtle), 14.0f);
                dl->AddRect(mn, mx, cardBorderU32(), 14.0f);

                ImGui::SetCursorScreenPos(ImVec2(mn.x + 14.0f, mn.y + 10.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ColTextPrimary, "%s", f.name.c_str());
                ImGui::PopFont();

                ImGui::SetCursorScreenPos(ImVec2(mn.x + 14.0f, mn.y + 30.0f));
                ImGui::PushTextWrapPos(mx.x - 130.0f);
                ImGui::TextColored(ColTextFaint, "%s", f.path.c_str());
                ImGui::PopTextWrapPos();

                ImGui::SetCursorScreenPos(ImVec2(mn.x + 14.0f, mn.y + 48.0f));
                ImGui::TextColored(ColTextMuted, "%s   |   %s",
                    formatSize(f.size).c_str(), formatMtime(f.mtime).c_str());

                ImGui::SetCursorScreenPos(ImVec2(mx.x - 110.0f, mn.y + 18.0f));
                if (gradientButton("##loadFound", "Load", ImVec2(96.0f, 32.0f), 12.0f)) {
                    uiState.pendingFilePath   = f.path;
                    uiState.triggeredOpenPath = true;
                }

                ImGui::SetCursorScreenPos(mn);
                ImGui::Dummy(ImVec2(rowW, 78.0f));
                ImGui::PopID();
            }

            cardEnd();
        }

    }

    void renderHelpPanel(data::TaskStore& /*store*/, UiState& uiState) {
        float avail = ImGui::GetContentRegionAvail().x;
        float gap   = 20.0f;
        float leftW = avail * 0.58f;
        float rightW = avail - leftW - gap;
        if (leftW < 480.0f) leftW = 480.0f;
        if (rightW < 320.0f) rightW = 320.0f;

        ImGui::BeginChild("##HelpLeft", ImVec2(leftW, 0.0f), false);
        renderGuides();
        ImGui::EndChild();

        ImGui::SameLine(0, gap);

        ImGui::BeginChild("##HelpRight", ImVec2(0.0f, 0.0f), false);
        renderFinder(uiState);
        ImGui::EndChild();
    }

}
