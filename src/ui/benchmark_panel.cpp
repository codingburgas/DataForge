#include "ui/benchmark_panel.h"
#include "ui/theme.h"
#include "ui/toast.h"
#include "logic/benchmark.h"
#include "imgui.h"

namespace ui {

    void renderBenchmarkPanel(UiState& uiState) {
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Sorting Benchmark");
        ImGui::PopFont();
        ImGui::TextColored(ColTextMuted, "Stress test the built-in sort implementations with the current workload size.");
        ImGui::Dummy(ImVec2(1.0f, 18.0f));

        float avail = ImGui::GetContentRegionAvail().x;
        float gap = 20.0f;
        float leftW = avail * 0.48f;
        if (leftW < 360.0f) leftW = 360.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
        ImGui::BeginChild("##BenchmarkControls", ImVec2(leftW, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding);
        float benchControlsW = ImGui::GetContentRegionAvail().x;
        ImVec2 cardMin = ImGui::GetCursorScreenPos();
        ImVec2 cardMax = ImVec2(cardMin.x + benchControlsW, cardMin.y + 232.0f);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        drawSoftShadow(dl, cardMin, cardMax, 20.0f);
        dl->AddRectFilled(cardMin, cardMax, cardBgU32(), 20.0f);
        dl->AddRect(cardMin, cardMax, cardBorderU32(), 20.0f);

        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 20.0f, cardMin.y + 22.0f));
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Run comparison");
        ImGui::PopFont();
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 20.0f, cardMin.y + 52.0f));
        ImGui::TextColored(ColTextFaint, "Quick sort should scale far better than bubble sort once the list grows.");

        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 20.0f, cardMin.y + 92.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextPrimary);
        ImGui::SliderInt("Item count", &uiState.benchmarkItemCount,
                         10, 50000, "%d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::PopStyleColor();

        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 20.0f, cardMin.y + 148.0f));
        if (gradientButton("##runBenchmark", "Run Benchmark", ImVec2(160.0f, 40.0f), 16.0f)) {
            unsigned seed = static_cast<unsigned>(
                std::chrono::system_clock::now().time_since_epoch().count());
            uiState.lastBenchmark =
                logic::benchmarkSortAlgorithms(uiState.benchmarkItemCount, seed);
            pushToast(uiState, "Benchmark complete.");
        }

        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 20.0f, cardMin.y + 198.0f));
        ImGui::TextColored(ColTextFaint, "Use the result to explain why the UI defaults to the faster algorithm.");
        ImGui::Dummy(ImVec2(1.0f, 248.0f));
        ImGui::EndChild();
        ImGui::PopStyleVar();

        ImGui::SameLine(0, gap);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
        ImGui::BeginChild("##BenchmarkResults", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding);
        dl = ImGui::GetWindowDrawList();

        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Results");
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(1.0f, 12.0f));

        if (!uiState.lastBenchmark.has_value()) {
            ImGui::TextColored(ColTextFaint, "No run yet. Set the item count and execute the benchmark.");
        } else {
            const logic::SortBenchmarkResult& r = *uiState.lastBenchmark;
            double maxMs = r.bubbleMs > r.quickMs ? r.bubbleMs : r.quickMs;
            if (maxMs <= 0.0) maxMs = 1.0;

            ImGui::TextColored(ColTextMuted, "Items tested: %d", r.itemCount);
            ImGui::Dummy(ImVec2(1.0f, 18.0f));

            struct RowDef {
                const char* label;
                double ms;
                ImU32 color;
            };
            RowDef rows[2] = {
                { "Bubble", r.bubbleMs, GradLeft },
                { "Quick",  r.quickMs,  IM_COL32(5, 150, 105, 255) }
            };

            for (const RowDef& row : rows) {
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ColTextPrimary, "%s", row.label);
                ImGui::PopFont();
                ImGui::SameLine(120.0f);
                ImVec2 pos = ImGui::GetCursorScreenPos();
                float width = ImGui::GetContentRegionAvail().x - 86.0f;
                float fill = width * static_cast<float>(row.ms / maxMs);
                dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + 10.0f),
                                  IM_COL32(241, 245, 249, 255), 99.0f);
                dl->AddRectFilled(pos, ImVec2(pos.x + fill, pos.y + 10.0f),
                                  row.color, 99.0f);
                ImGui::Dummy(ImVec2(width, 10.0f));
                ImGui::SameLine(120.0f + width + 10.0f);
                ImGui::TextColored(ColTextMuted, "%.2f ms", row.ms);
                ImGui::Dummy(ImVec2(1.0f, 14.0f));
            }

            double ratio = r.quickMs > 0.0 ? r.bubbleMs / r.quickMs : 0.0;
            ImVec2 noteMin = ImGui::GetCursorScreenPos();
            ImVec2 noteMax = ImVec2(noteMin.x + ImGui::GetContentRegionAvail().x, noteMin.y + 118.0f);
            dl->AddRectFilled(noteMin, noteMax, cardBgU32(), 18.0f);
            dl->AddRect(noteMin, noteMax, cardBorderU32(), 18.0f);
            ImGui::SetCursorScreenPos(ImVec2(noteMin.x + 18.0f, noteMin.y + 18.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextPrimary, "Takeaway");
            ImGui::PopFont();
            ImGui::SetCursorScreenPos(ImVec2(noteMin.x + 18.0f, noteMin.y + 46.0f));
            ImGui::TextColored(ColTextMuted,
                               "Quick sort is %.1fx faster on this run.",
                               ratio);
            ImGui::SetCursorScreenPos(ImVec2(noteMin.x + 18.0f, noteMin.y + 74.0f));
            ImGui::TextColored(ColTextFaint,
                               "Bubble sort is still useful here as a teaching tool, but it should not be the default path.");
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

}
