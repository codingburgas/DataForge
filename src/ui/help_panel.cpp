#include "ui/help_panel.h"
#include "ui/theme.h"
#include "ui/i18n.h"
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
                    for (char& c : ext) {
                        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
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
            if (t == 0) return tr(K_HP_UNKNOWN);
            std::tm local{};
            if (localtime_s(&local, &t) != 0) return tr(K_HP_UNKNOWN);
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
            sectionHeading(tr(K_HP_GUIDE_HEADING), tr(K_HP_GUIDE_SUB));

            guideEntry(tr(K_HP_T_SIDEBAR),    tr(K_HP_B_SIDEBAR));
            guideEntry(tr(K_HP_T_TOPBAR),     tr(K_HP_B_TOPBAR));
            guideEntry(tr(K_HP_T_OVERVIEW),   tr(K_HP_B_OVERVIEW));
            guideEntry(tr(K_HP_T_MYTASKS),    tr(K_HP_B_MYTASKS));
            guideEntry(tr(K_HP_T_TREE),       tr(K_HP_B_TREE));
            guideEntry(tr(K_HP_T_TABLE),      tr(K_HP_B_TABLE));
            guideEntry(tr(K_HP_T_DETAILS),    tr(K_HP_B_DETAILS));
            guideEntry(tr(K_HP_T_ANALYTICS),  tr(K_HP_B_ANALYTICS));
            guideEntry(tr(K_HP_T_BENCHMARK),  tr(K_HP_B_BENCHMARK));
            guideEntry(tr(K_HP_T_STATUS),     tr(K_HP_B_STATUS));
            guideEntry(tr(K_HP_T_MENU),       tr(K_HP_B_MENU));
            guideEntry(tr(K_HP_T_VOICE),      tr(K_HP_B_VOICE));
            guideEntry(tr(K_HP_T_KEYS),       tr(K_HP_B_KEYS));

            cardEnd();
        }

        void renderFinder(UiState& uiState) {
            cardBegin("##HelpFinder", 0.0f);
            sectionHeading(tr(K_HP_FINDER_HEADING), tr(K_HP_FINDER_SUB));

            ScanState& s = scanState();

            if (gradientButton("##scanDocs", tr(K_HP_SCAN_BTN), ImVec2(170.0f, 36.0f), 14.0f)) {
                runScan();
                if (!s.docsExist) {
                    pushToast(uiState, tr(K_TOAST_DOCS_NOT_FOUND));
                } else {
                    char msg[96];
                    std::snprintf(msg, sizeof(msg), tr(K_TOAST_SCAN_DONE_FMT), s.results.size());
                    pushToast(uiState, msg);
                }
            }

            ImGui::SameLine(0, 12.0f);
            if (s.ran) {
                ImGui::TextColored(ColTextFaint, tr(K_HP_SEARCHED_FMT),
                    s.docsPath.empty() ? tr(K_HP_NO_DOCS_PATH) : s.docsPath.c_str());
            } else {
                ImGui::TextColored(ColTextFaint, "%s", tr(K_HP_CLICK_TO_SEARCH));
            }

            ImGui::Dummy(ImVec2(1.0f, 10.0f));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(1.0f, 8.0f));

            if (!s.ran) {
                ImGui::TextColored(ColTextFaint, "%s", tr(K_HP_RESULTS_HINT));
                cardEnd();
                return;
            }

            if (!s.docsExist) {
                ImGui::TextColored(HEX(0xDC2626), "%s", tr(K_HP_DOCS_MISSING));
                cardEnd();
                return;
            }

            if (s.results.empty()) {
                ImGui::TextColored(ColTextFaint, "%s", tr(K_HP_NO_DFTASKS));
                cardEnd();
                return;
            }

            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextSecondary, tr(K_HP_FOUND_FMT), s.results.size());
            ImGui::PopFont();
            ImGui::Dummy(ImVec2(1.0f, 8.0f));

            int rowIdx = 0;
            for (const FoundFile& f : s.results) {
                ImGui::PushID(rowIdx++);
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
                if (gradientButton("##loadFound", tr(K_HP_LOAD), ImVec2(96.0f, 32.0f), 12.0f)) {
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
