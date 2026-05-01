#include "ui/benchmark_panel.h"
#include "ui/toast.h"
#include "logic/benchmark.h"
#include "imgui.h"

namespace ui {

    void renderBenchmarkPanel(UiState& uiState) {
        if (!uiState.showBenchmarkPanel) {
            return;
        }
        ImGui::SetNextWindowSize(ImVec2(420.0f, 280.0f), ImGuiCond_Appearing);
        if (!ImGui::Begin("Benchmark", &uiState.showBenchmarkPanel)) {
            ImGui::End();
            return;
        }
        ImGui::Text("Compare Bubble Sort and Quick Sort on synthetic tasks.");
        ImGui::Spacing();
        ImGui::SliderInt("Item count",
                         &uiState.benchmarkItemCount,
                         10, 50000,
                         "%d",
                         ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::Button("Run benchmark", ImVec2(140.0f, 0.0f))) {
            unsigned seed = static_cast<unsigned>(
                std::chrono::system_clock::now().time_since_epoch().count());
            logic::SortBenchmarkResult r =
                logic::benchmarkSortAlgorithms(uiState.benchmarkItemCount, seed);
            uiState.lastBenchmark = r;
            pushToast(uiState, "Benchmark complete.");
        }
        ImGui::Separator();
        if (!uiState.lastBenchmark.has_value()) {
            ImGui::TextDisabled("(no run yet)");
        } else {
            const logic::SortBenchmarkResult& r = *uiState.lastBenchmark;
            ImGui::Text("Items: %d", r.itemCount);
            ImGui::Text("Bubble Sort (priority): %.3f ms", r.bubbleMs);
            ImGui::Text("Quick Sort  (deadline): %.3f ms", r.quickMs);
            double maxMs = r.bubbleMs > r.quickMs ? r.bubbleMs : r.quickMs;
            if (maxMs <= 0.0) {
                maxMs = 1.0;
            }
            ImGui::Text("Bubble:");
            ImGui::SameLine();
            ImGui::ProgressBar(static_cast<float>(r.bubbleMs / maxMs),
                               ImVec2(-1.0f, 0.0f));
            ImGui::Text("Quick: ");
            ImGui::SameLine();
            ImGui::ProgressBar(static_cast<float>(r.quickMs / maxMs),
                               ImVec2(-1.0f, 0.0f));
            ImGui::Spacing();
            ImGui::TextWrapped(
                "Bubble Sort is O(n^2) and degrades quickly with more items; "
                "Quick Sort is O(n log n) on average. The gap grows with n.");
        }
        ImGui::End();
    }

}
