<div align="center">

<img src="https://i.imgur.com/kIk8K5W.png" alt="DataForge Logo" width="700" />

**Forge your workflow - a native Windows task manager with hierarchical tasks, hand-rolled algorithms, and a Dear ImGui interface.**

[![Language](https://img.shields.io/github/languages/top/codingburgas/DataForge?style=flat-square&color=00599C&logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![Last Commit](https://img.shields.io/github/last-commit/codingburgas/DataForge?style=flat-square&color=f59e0b)](https://github.com/codingburgas/DataForge/commits/main)
[![Repo Size](https://img.shields.io/github/repo-size/codingburgas/DataForge?style=flat-square&color=8b5cf6)](https://github.com/codingburgas/DataForge)
[![Contributors](https://img.shields.io/github/contributors/codingburgas/DataForge?style=flat-square&color=10b981)](https://github.com/codingburgas/DataForge/graphs/contributors)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/20)
[![Platform](https://img.shields.io/badge/platform-Windows-0078D6?style=flat-square&logo=windows&logoColor=white)](#prerequisites)
[![IDE](https://img.shields.io/badge/IDE-Visual%20Studio%202026-5C2D91?style=flat-square&logo=visualstudio&logoColor=white)](https://visualstudio.microsoft.com/)
[![Status](https://img.shields.io/badge/status-in%20development-yellow?style=flat-square)](#)

</div>

---
# 
## 📑 Table of Contents

1. [🎯 About The Project](#-about-the-project)
2. [✨ Features](#-features)
3. [🚀 Installation & Run](#-installation--run)
   - [📋 Prerequisites](#-prerequisites)
   - [📦 Installation](#-installation)
   - [▶️ Running the App](#-running-the-app)
4. [📂 Project Structure](#-project-structure)
5. [🔍 Example Usage / How It Works](#-example-usage--how-it-works)
6. [🛠️ Tech Stack](#-tech-stack)
7. [👥 Team](#-team)
8. [🙏 Acknowledgements](#-acknowledgements)

---
# 
## 🎯 About The Project

**DataForge** is a single-window desktop **task management system** written in procedural C++20 and rendered with [Dear ImGui](https://github.com/ocornut/imgui). It treats work the way humans actually think about it — as a **tree of tasks and subtasks** — rather than a flat checklist.

That single design choice turns every requirement of the brief into something that feels native to the domain:

- **Recursion** sums estimated time across a subtree, cascades deletions, and computes weighted completion — not a toy factorial.
- **Sorting** is offered in two hand-rolled flavours (Bubble & Quick), user-selectable, with a live benchmark panel that shows *why* algorithm choice matters.
- **Searching** pairs a substring linear scan over titles (what users expect from a search bar) with a recursive binary search over a cached ID index (the fast internal lookup).

Under the hood, DataForge is built on a strict **three-tier architecture** — *presentation → logic → data* — with one-way calls only. The presentation layer never touches the data layer directly; every mutation flows through the logic tier. No OOP, no classes, no inheritance — just structs and free functions passed explicitly between layers.
### 🏗️ Architecture at a glance

```
main.cpp  ──▶  presentation  ──▶  logic  ──▶  data
                  (Dear ImGui)     (rules)    (CRUD + I/O)
```
### 💎 Why it stands out

- **Zero external dependencies** beyond the Windows SDK and vendored ImGui sources — no vcpkg, no NuGet, no package manager gymnastics.
- **Atomic save** via write-and-rename protects user data from mid-save crashes.
- **Custom `.dftasks` text format** that you can read, diff, and edit in Notepad.

---
## ✨ Features

| Feature | Description |
|---|---|
| **Hierarchical Tasks** | Every task can have unlimited subtasks, forming a tree encoded by `parentId`. |
| **Dual Sort Algorithms** | Bubble Sort (iterative, by priority DESC) and Quick Sort (recursive, by deadline ASC) — user-selectable. |
| **Sort Benchmark Panel** | Generates *N* synthetic tasks and times both algorithms head-to-head in milliseconds. |
| **Dual Search** | Case-insensitive substring linear search on titles plus recursive binary search by ID. |
| **Recursive Aggregates** | Five domain-meaningful tree functions: total estimated time, weighted completion, descendant count, max depth, descendant-ID collection. |
| **Cascade Delete** | Removing a task recursively removes its subtree after a confirmation dialog that shows how many descendants will go. |
| **Filter + Search Compose** | Filter by status or priority first, then substring-match within the narrowed set. |
| **Statistics Panel** | Counts by priority, counts by status, overdue tasks, and total estimated time across all roots. |
| **Urgency Coloring** | Deadline cells tint red when overdue, amber when ≤ 3 days away, green when comfortable. |
| **Input Validation** | Invalid drafts are caught before they reach the store; errors show inline and as a toast. |
| **Single-Step Undo** | Snapshot-based undo restores the store after a destructive operation — procedural-friendly, not command-based. |
| **Keyboard Shortcuts** | `Ctrl+N` new, `Ctrl+S` save, `Ctrl+O` open, `Ctrl+F` focus search, `Ctrl+Z` undo, `Del` remove selected, arrow keys to navigate. |
| **Light / Dark Theme** | ImGui theme toggle persisted in `UiState`. |
| **Atomic Persistence** | Human-readable `.dftasks` format written via write-and-rename — no corruption on crash. |
| **Dirty-Flag UX** | Window title marks unsaved changes; exit confirms if work is pending. |
| **Status Bar** | Live total count, filtered count, last action, and save state — always visible. |

---
## 🚀 Installation & Run

### 📋 Prerequisites

| Requirement | Version / Notes |
|---|---|
| **OS** | Windows 10 / 11 (64-bit) |
| **IDE** | Visual Studio 2026 (or 2022 with the v143 toolset) |
| **Workloads** | *Desktop development with C++* (includes MSVC, Windows SDK, and the DirectX headers) |
| **Toolset** | MSVC with `/std:c++20` |
| **Git** | Any recent version |

No external package manager is required. Dear ImGui is vendored into `vendor/imgui/`.

### 📦 Installation

```bash
# 1. Clone the repository
git clone https://github.com/codingburgas/DataForge.git
cd DataForge

# 2. Open the solution in Visual Studio 2026
start DataForge.sln
```

Inside Visual Studio:

1. Select the `x64` platform and either the `Debug` or `Release` configuration.
2. Build the solution (`Ctrl + Shift + B`).
3. Run with `F5` (debug) or `Ctrl + F5` (run without debugging).

### ▶️ Running the App

On first launch DataForge looks for `Resources/sample_tasks.dftasks` and loads it if present, so the app opens with realistic demo data. Use **File → New** to start from a blank store, or **File → Open…** to load your own `.dftasks` file.

---
## 📂 Project Structure

```text
DataForge/
├── DataForge.slnx                    # Visual Studio solution
├── DataForge.vcxproj                # Main application project
├── DataForge.vcxproj.filters        # Source grouping for the IDE
│
├── src/                             # Application source (strict 3-tier split)
│   ├── main.cpp                     # Win32 + D3D11 + ImGui bootstrap, main loop
│   ├── presentation layer        # Dear ImGui rendering & UiState (calls logic only)
│   ├── logic layer               # Business rules, algorithms, validation
│   └── data layer                # Task / TaskStore / Date structs, CRUD, file I/O
│
├── vendor/
│   └── imgui/                       # Vendored Dear ImGui sources (pinned SHA in VERSION.txt)
│       ├── imgui*.cpp / .h
│       └── backends/
│           ├── imgui_impl_win32.*
│           └── imgui_impl_dx11.*
│
│
├── Documentation/                   # Architecture diagram and sprint reports
│   ├── DOCX Documentation
│   ├── PPTX Presentation
│   └── QA Report
│
│
├── README.md                        # You are here
└── .gitignore
```

---
## ⌨️ Keyboard shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl + N` | New task |
| `Ctrl + S` | Save store |
| `Ctrl + O` | Open store |
| `Ctrl + F` | Focus search |
| `Ctrl + Z` | Undo last destructive action |
| `Delete` | Remove selected task |
| `↑ / ↓` | Navigate task list |

---
## 🛠️ Tech Stack

| Layer | Choice | Why |
|---|---|---|
| **Language** | C++20 | `std::optional`, structured bindings, `<=>`, `std::format`, `std::span`, designated initialisers — modern without being exotic. |
| **Compiler** | MSVC (Visual Studio 2026), `/std:c++20`, `/W4 /WX` in Release | First-party Microsoft toolchain; no cross-compilation friction. |
| **GUI** | [Dear ImGui](https://github.com/ocornut/imgui) (vendored) | Immediate-mode, single-window desktop apps in ~20 files. Pinned commit SHA in `vendor/imgui/VERSION.txt`. |
| **Rendering Backend** | Win32 + Direct3D 11 | Zero external package managers — every header ships with the Windows SDK. |
| **Build System** | Visual Studio `.sln` / `.vcxproj` | Single solution, one project for the app, optional test project. No NuGet, no vcpkg. |
| **Persistence** | Custom `.dftasks` line-based text format | Human-readable, diff-friendly, zero library dependencies. |
| **Linker Dependencies** | `d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib` | Windows SDK only. |

---

## 👥 Team

| Name | Role |
|---|---|
| **Atanas Todorov** | Scrum Trainer & Project Lead  |
| **Dimitar Yanakiev** | Back-End Developer |
| **Georgi Dinkov** | Back-End Developer |
| **Stoqn Savakov** | QA Engineer |
| **Dimitar Dimitrov** | QA & F-E Developer |

---
## 🙏 Acknowledgements

- **[Dear ImGui](https://github.com/ocornut/imgui)** by Omar Cornut — the immediate-mode GUI library at the heart of DataForge.
- **[Shields.io](https://shields.io/)** — for all tech badges above.
- **[Placehold.co](https://placehold.co/)** — placeholder imagery during early development.
- **Microsoft Visual Studio & the Windows SDK** — the C++20 toolchain and D3D11 backend.
- Every author of the classic algorithms textbooks whose Bubble Sort, Quick Sort, and Binary Search we re-implement by hand in this repository.

---

<div align="center">

*Forged with C++20 and much ❤️ for the Windows desktop.*

</div>
