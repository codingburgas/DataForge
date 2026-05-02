#include "data/persistence.h"
#include "data/date.h"
#include "data/store.h"

namespace data {

    static const char* FILE_MAGIC  = "# DataForge task store v1";
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
        TaskStore parsed;
        if (!storeFromText(text, parsed, errorMessage)) {
            return false;
        }
        store          = parsed;
        store.filePath = path;
        store.dirty    = false;
        return true;
    }

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
