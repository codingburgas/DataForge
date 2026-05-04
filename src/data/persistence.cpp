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
            ss << RECORD_SEP << "\n";
        }
        out = ss.str();
        return true;
    }

    // Apply one parsed key=value pair to the in-progress task. Unknown
    // keys are ignored on purpose so future versions can add fields
    // without breaking older builds.
    static void applyKeyValue(Task& t,
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
            int s = std::atoi(value.c_str());
            if (s < STATUS_TODO) s = STATUS_TODO;
            if (s > STATUS_BLOCKED) s = STATUS_BLOCKED;
            t.status = static_cast<Status>(s);
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

    // Streaming parser. Walks the file line by line maintaining a tiny
    // state machine: header → first separator → record body → separator
    // → next record. Empty lines and `#` comments are tolerated anywhere
    // so the format stays friendly to manual edits.
    bool storeFromText(const std::string& text,
                       TaskStore& store,
                       std::string& errorMessage) {
        TaskStore tmp = createEmptyStore();
        std::istringstream ss(text);
        std::string line;
        bool sawMagic        = false;
        bool inHeader        = true;
        bool recordOpen      = false;
        Task current         = defaultTask();
        int  lineNo          = 0;

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
            if (line == RECORD_SEP) {
                if (inHeader) {
                    inHeader   = false;
                    recordOpen = true;
                    current    = defaultTask();
                } else if (recordOpen) {
                    // Only commit records that actually got an id —
                    // otherwise an empty trailing record block would
                    // create a phantom task.
                    if (current.id > 0) {
                        tmp.tasks.push_back(current);
                    }
                    current    = defaultTask();
                    recordOpen = true;
                }
                continue;
            }
            std::string key;
            std::string value;
            if (!splitKeyValue(line, key, value)) {
                continue;
            }
            if (inHeader) {
                if (key == "nextId") {
                    tmp.nextId = std::atoi(value.c_str());
                }
            } else {
                applyKeyValue(current, key, value);
            }
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
