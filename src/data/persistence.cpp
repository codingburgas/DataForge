#include "data/persistence.h"
#include "data/date.h"
#include "data/store.h"

namespace data {

    // The first line of every .dftasks file. We refuse to load anything
    // that does not start with this exact string so the user cannot
    // accidentally point the app at an unrelated text file.
    static const char* FILE_MAGIC  = "# DataForge task store v1";
    // Separates the header block from records, and one record from the
    // next. Chosen so the file remains diff-friendly in plain text tools.
    static const char* RECORD_SEP  = "---";
    // Section markers introduced after the original task list. Older
    // files without these markers still load (history/productivity stay
    // empty), keeping backward compatibility intact.
    static const char* SECTION_HISTORY      = "===HISTORY===";
    static const char* SECTION_PRODUCTIVITY = "===PRODUCTIVITY===";

    static std::string trimRight(const std::string& s) {
        std::size_t end = s.size();
        while (end > 0) {
            char c = s[end - 1];
            if (c == '\r' || c == '\n' || c == ' ' || c == '\t') {
                end -= 1;
            } else {
                break;
            }
        }
        return s.substr(0, end);
    }

    // Lines are key=value. Anything before the first '=' is the key,
    // everything after is the value (including any '=' inside it).
    static bool splitKeyValue(const std::string& line,
                              std::string& key,
                              std::string& value) {
        std::size_t eq = line.find('=');
        if (eq == std::string::npos) {
            return false;
        }
        key   = line.substr(0, eq);
        value = line.substr(eq + 1);
        return true;
    }

    // Backslash-escape so titles/descriptions can contain newlines and
    // backslashes without breaking the line-based format. Only three
    // sequences exist (\\ \n \r) which keeps the unescape side trivial.
    static std::string escapeValue(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '\\') {
                out += "\\\\";
            } else if (c == '\n') {
                out += "\\n";
            } else if (c == '\r') {
                out += "\\r";
            } else {
                out += c;
            }
        }
        return out;
    }

    static std::string unescapeValue(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (std::size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                char next = s[i + 1];
                if (next == '\\') { out += '\\'; i += 1; continue; }
                if (next == 'n')  { out += '\n'; i += 1; continue; }
                if (next == 'r')  { out += '\r'; i += 1; continue; }
            }
            out += s[i];
        }
        return out;
    }

    // Header block first (magic + nextId), then one record per task,
    // each terminated by the RECORD_SEP line. Keeping the format
    // line-oriented means small edits show up as small diffs in git.
    bool storeToText(const TaskStore& store, std::string& out) {
        std::ostringstream ss;
        ss << FILE_MAGIC << "\n";
        ss << "nextId=" << store.nextId << "\n";
        ss << RECORD_SEP << "\n";
        for (const Task& t : store.tasks) {
            ss << "id="                << t.id               << "\n";
            ss << "parent="            << t.parentId         << "\n";
            ss << "title="             << escapeValue(t.title)        << "\n";
            ss << "description="       << escapeValue(t.description)  << "\n";
            ss << "priority="          << static_cast<int>(t.priority) << "\n";
            ss << "status="            << static_cast<int>(t.status)   << "\n";
            ss << "deadline="          << formatDate(t.deadline)  << "\n";
            ss << "estimatedMinutes="  << t.estimatedMinutes << "\n";
            ss << "actualMinutes="     << t.actualMinutes    << "\n";
            ss << "createdAt="         << formatDate(t.createdAt) << "\n";
            ss << "updatedAt="         << formatDate(t.updatedAt) << "\n";
            for (const Decision& dec : t.decisions) {
                ss << "decision_question=" << escapeValue(dec.question) << "\n";
                ss << "decision_chosen="   << dec.chosenIndex << "\n";
                for (const DecisionOption& opt : dec.options) {
                    ss << "decision_option=" << static_cast<int>(opt.effect)
                       << "|" << escapeValue(opt.text) << "\n";
                }
                ss << "decision_end=" << "\n";
            }
            ss << RECORD_SEP << "\n";
        }

        ss << SECTION_HISTORY << "\n";
        ss << "nextHistoryId=" << store.nextHistoryId << "\n";
        ss << RECORD_SEP << "\n";
        for (const HistoryEntry& h : store.history) {
            ss << "hid="      << h.id                            << "\n";
            ss << "htime="    << escapeValue(h.timestamp)        << "\n";
            ss << "haction="  << static_cast<int>(h.action)      << "\n";
            ss << "htask="    << h.taskId                        << "\n";
            ss << "hsummary=" << escapeValue(h.summary)          << "\n";
            ss << RECORD_SEP << "\n";
        }

        ss << SECTION_PRODUCTIVITY << "\n";
        ss << "xp="           << store.productivity.xp           << "\n";
        ss << "totalDone="    << store.productivity.totalDone    << "\n";
        ss << "totalCreated=" << store.productivity.totalCreated << "\n";
        ss << "totalEdited="  << store.productivity.totalEdited  << "\n";
        for (const std::string& b : store.productivity.earnedBadges) {
            ss << "badge=" << escapeValue(b) << "\n";
        }
        for (const std::string& d : store.productivity.completionDates) {
            ss << "completion=" << escapeValue(d) << "\n";
        }
        out = ss.str();
        return true;
    }

    static Status clampStatus(int s) {
        if (s < STATUS_TODO)    s = STATUS_TODO;
        if (s > STATUS_BLOCKED) s = STATUS_BLOCKED;
        return static_cast<Status>(s);
    }

    // Apply one parsed key=value pair to the in-progress task. Unknown
    // keys are ignored on purpose so future versions can add fields
    // without breaking older builds. `pendingDecision` is the decision
    // record currently being assembled; the caller flushes it on
    // `decision_end=`.
    static void applyKeyValue(Task& t,
                              Decision& pendingDecision,
                              bool& hasPendingDecision,
                              const std::string& key,
                              const std::string& value) {
        if (key == "id") {
            t.id = std::atoi(value.c_str());
        } else if (key == "parent") {
            t.parentId = std::atoi(value.c_str());
        } else if (key == "title") {
            t.title = unescapeValue(value);
        } else if (key == "description") {
            t.description = unescapeValue(value);
        } else if (key == "priority") {
            // Clamp into the enum range so a tampered file cannot put
            // the store into an invalid state the validator never sees.
            int p = std::atoi(value.c_str());
            if (p < PRIORITY_LOW) p = PRIORITY_LOW;
            if (p > PRIORITY_CRITICAL) p = PRIORITY_CRITICAL;
            t.priority = static_cast<Priority>(p);
        } else if (key == "status") {
            t.status = clampStatus(std::atoi(value.c_str()));
        } else if (key == "deadline") {
            Date d{};
            if (parseDate(value, d)) {
                t.deadline = d;
            }
        } else if (key == "estimatedMinutes") {
            t.estimatedMinutes = std::atoi(value.c_str());
        } else if (key == "actualMinutes") {
            t.actualMinutes = std::atoi(value.c_str());
        } else if (key == "createdAt") {
            Date d{};
            if (parseDate(value, d)) {
                t.createdAt = d;
            }
        } else if (key == "updatedAt") {
            Date d{};
            if (parseDate(value, d)) {
                t.updatedAt = d;
            }
        } else if (key == "decision_question") {
            pendingDecision = Decision{};
            pendingDecision.question     = unescapeValue(value);
            pendingDecision.chosenIndex  = -1;
            hasPendingDecision           = true;
        } else if (key == "decision_chosen") {
            if (hasPendingDecision) {
                pendingDecision.chosenIndex = std::atoi(value.c_str());
            }
        } else if (key == "decision_option") {
            if (hasPendingDecision) {
                std::size_t pipe = value.find('|');
                if (pipe != std::string::npos) {
                    DecisionOption opt;
                    opt.effect = clampStatus(
                        std::atoi(value.substr(0, pipe).c_str()));
                    opt.text   = unescapeValue(value.substr(pipe + 1));
                    pendingDecision.options.push_back(opt);
                }
            }
        } else if (key == "decision_end") {
            if (hasPendingDecision) {
                t.decisions.push_back(pendingDecision);
                pendingDecision    = Decision{};
                hasPendingDecision = false;
            }
        }
    }

    static void applyHistoryKeyValue(HistoryEntry& h,
                                     const std::string& key,
                                     const std::string& value) {
        if (key == "hid") {
            h.id = std::atoi(value.c_str());
        } else if (key == "htime") {
            h.timestamp = unescapeValue(value);
        } else if (key == "haction") {
            int a = std::atoi(value.c_str());
            if (a < HIST_CREATE)        a = HIST_CREATE;
            if (a > HIST_DECISION)      a = HIST_DECISION;
            h.action = static_cast<HistoryAction>(a);
        } else if (key == "htask") {
            h.taskId = std::atoi(value.c_str());
        } else if (key == "hsummary") {
            h.summary = unescapeValue(value);
        }
    }

    static void applyProductivityKeyValue(ProductivityState& p,
                                          const std::string& key,
                                          const std::string& value) {
        if (key == "xp") {
            p.xp = std::atoi(value.c_str());
        } else if (key == "totalDone") {
            p.totalDone = std::atoi(value.c_str());
        } else if (key == "totalCreated") {
            p.totalCreated = std::atoi(value.c_str());
        } else if (key == "totalEdited") {
            p.totalEdited = std::atoi(value.c_str());
        } else if (key == "badge") {
            p.earnedBadges.push_back(unescapeValue(value));
        } else if (key == "completion") {
            p.completionDates.push_back(unescapeValue(value));
        }
    }

    // Default values used when a record is missing fields. id=0 acts as a
    // "not yet assigned" marker — the loader skips records whose id never
    // got set, so a half-written record can't make it into the store.
    static Task defaultTask() {
        Task t{};
        t.id               = 0;
        t.parentId         = -1;
        t.priority         = PRIORITY_MEDIUM;
        t.status           = STATUS_TODO;
        t.deadline         = makeZeroDate();
        t.estimatedMinutes = 0;
        t.actualMinutes    = 0;
        t.createdAt        = makeZeroDate();
        t.updatedAt        = makeZeroDate();
        return t;
    }

    enum ParseSection {
        SECT_HEADER,
        SECT_TASKS,
        SECT_HISTORY_HEADER,
        SECT_HISTORY,
        SECT_PRODUCTIVITY
    };

    // Streaming parser. Walks the file line by line maintaining a small
    // state machine across sections: header -> tasks -> history header
    // -> history records -> productivity. Empty lines and `#` comments
    // are tolerated anywhere so the format stays friendly to manual
    // edits.
    bool storeFromText(const std::string& text,
                       TaskStore& store,
                       std::string& errorMessage) {
        TaskStore tmp = createEmptyStore();
        std::istringstream ss(text);
        std::string line;

        bool sawMagic         = false;
        ParseSection section  = SECT_HEADER;
        Task currentTask      = defaultTask();
        bool taskOpen         = false;
        Decision pendingDec{};
        bool hasPendingDec    = false;
        HistoryEntry currentHist{};
        bool histOpen         = false;
        int  lineNo           = 0;

        while (std::getline(ss, line)) {
            lineNo += 1;
            line    = trimRight(line);

            if (line.empty()) {
                continue;
            }
            if (!sawMagic) {
                if (line != FILE_MAGIC) {
                    errorMessage = "missing DataForge header at line 1";
                    return false;
                }
                sawMagic = true;
                continue;
            }
            if (line.rfind('#', 0) == 0) {
                continue;
            }

            if (line == SECTION_HISTORY) {
                if (taskOpen && currentTask.id > 0) {
                    tmp.tasks.push_back(currentTask);
                }
                taskOpen        = false;
                hasPendingDec   = false;
                section         = SECT_HISTORY_HEADER;
                continue;
            }
            if (line == SECTION_PRODUCTIVITY) {
                section = SECT_PRODUCTIVITY;
                continue;
            }

            if (line == RECORD_SEP) {
                if (section == SECT_HEADER) {
                    section      = SECT_TASKS;
                    taskOpen     = true;
                    currentTask  = defaultTask();
                } else if (section == SECT_TASKS) {
                    if (taskOpen && currentTask.id > 0) {
                        tmp.tasks.push_back(currentTask);
                    }
                    currentTask = defaultTask();
                    taskOpen    = true;
                } else if (section == SECT_HISTORY_HEADER) {
                    section     = SECT_HISTORY;
                    currentHist = HistoryEntry{};
                    histOpen    = true;
                } else if (section == SECT_HISTORY) {
                    if (histOpen && currentHist.id > 0) {
                        tmp.history.push_back(currentHist);
                    }
                    currentHist = HistoryEntry{};
                    histOpen    = true;
                }
                continue;
            }

            std::string key;
            std::string value;
            if (!splitKeyValue(line, key, value)) {
                continue;
            }

            switch (section) {
                case SECT_HEADER:
                    if (key == "nextId") {
                        tmp.nextId = std::atoi(value.c_str());
                    }
                    break;
                case SECT_TASKS:
                    applyKeyValue(currentTask, pendingDec,
                                  hasPendingDec, key, value);
                    break;
                case SECT_HISTORY_HEADER:
                    if (key == "nextHistoryId") {
                        tmp.nextHistoryId = std::atoi(value.c_str());
                    }
                    break;
                case SECT_HISTORY:
                    applyHistoryKeyValue(currentHist, key, value);
                    break;
                case SECT_PRODUCTIVITY:
                    applyProductivityKeyValue(tmp.productivity, key, value);
                    break;
            }
        }

        // Flush any unterminated trailing record. Common when the file
        // does not end with a record separator after the last task.
        if (section == SECT_TASKS && taskOpen && currentTask.id > 0) {
            tmp.tasks.push_back(currentTask);
        }
        if (section == SECT_HISTORY && histOpen && currentHist.id > 0) {
            tmp.history.push_back(currentHist);
        }

        // If the header didn't supply nextId (or it was bogus), recover
        // by taking max(existing id) + 1 so newly created tasks can't
        // collide with loaded ones.
        if (tmp.nextId < 1) {
            int maxId = 0;
            for (const Task& t : tmp.tasks) {
                if (t.id > maxId) {
                    maxId = t.id;
                }
            }
            tmp.nextId = maxId + 1;
        }
        if (tmp.nextHistoryId < 1) {
            int maxHid = 0;
            for (const HistoryEntry& h : tmp.history) {
                if (h.id > maxHid) {
                    maxHid = h.id;
                }
            }
            tmp.nextHistoryId = maxHid + 1;
        }

        store = tmp;
        store.dirty = false;
        errorMessage.clear();
        return true;
    }

    bool loadStoreFromFile(TaskStore& store,
                           const std::string& path,
                           std::string& errorMessage) {
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            errorMessage = "could not open file: " + path;
            return false;
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        std::string text = ss.str();
        // Parse into a temporary first so a failed load never partially
        // overwrites the in-memory store the user is working in.
        TaskStore parsed;
        if (!storeFromText(text, parsed, errorMessage)) {
            return false;
        }
        store          = parsed;
        store.filePath = path;
        store.dirty    = false;
        return true;
    }

    // Atomic save: write to a sibling .tmp file, fsync via ofstream's
    // close, then rename over the original with MoveFileEx. If the app
    // (or the OS) crashes mid-write, the original file is still intact —
    // either the rename happened or it didn't.
    bool saveStoreToFile(TaskStore& store,
                         const std::string& path,
                         std::string& errorMessage) {
        std::string text;
        if (!storeToText(store, text)) {
            errorMessage = "failed to serialise store";
            return false;
        }
        std::string tmpPath = path + ".tmp";
        {
            std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
            if (!out) {
                errorMessage = "could not open temp file: " + tmpPath;
                return false;
            }
            out.write(text.data(), static_cast<std::streamsize>(text.size()));
            if (!out) {
                errorMessage = "write failed: " + tmpPath;
                return false;
            }
            out.flush();
        }
        // MOVEFILE_REPLACE_EXISTING overwrites the original; WRITE_THROUGH
        // forces the rename through the disk cache so a power-loss after
        // this returns success cannot lose the new file.
        BOOL ok = MoveFileExA(tmpPath.c_str(),
                              path.c_str(),
                              MOVEFILE_REPLACE_EXISTING |
                              MOVEFILE_WRITE_THROUGH);
        if (!ok) {
            DWORD err = GetLastError();
            std::ostringstream msg;
            msg << "rename failed (error " << err << "): " << path;
            errorMessage = msg.str();
            return false;
        }
        store.filePath = path;
        store.dirty    = false;
        errorMessage.clear();
        return true;
    }

}
