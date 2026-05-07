#include "logic/productivity.h"

namespace logic {

    static const char* BADGE_FIRST_TASK = "first_task";
    static const char* BADGE_TEN_DONE   = "ten_done";
    static const char* BADGE_FIFTY_DONE = "fifty_done";
    static const char* BADGE_STREAK_3   = "streak_3";
    static const char* BADGE_STREAK_7   = "streak_7";

    static void awardIfMissing(data::ProductivityState& s, const char* id) {
        for (const std::string& existing : s.earnedBadges) {
            if (existing == id) {
                return;
            }
        }
        s.earnedBadges.emplace_back(id);
    }

    bool hasBadge(const data::ProductivityState& s, const std::string& id) {
        for (const std::string& existing : s.earnedBadges) {
            if (existing == id) {
                return true;
            }
        }
        return false;
    }

    static int dayOfYear(int y, int m, int d) {
        static const int CUM[12] = { 0, 31, 59, 90, 120, 151,
                                     181, 212, 243, 273, 304, 334 };
        int doy = CUM[m - 1] + d;
        bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
        if (leap && m > 2) {
            doy += 1;
        }
        return doy;
    }

    // Returns absolute day index since year 0 for ordering. Approximate
    // (uses 365.25 average); only the difference matters for streaks.
    static long long absoluteDay(const std::string& iso) {
        if (iso.size() < 10) {
            return 0;
        }
        int y = std::atoi(iso.substr(0, 4).c_str());
        int m = std::atoi(iso.substr(5, 2).c_str());
        int d = std::atoi(iso.substr(8, 2).c_str());
        if (m < 1 || m > 12 || d < 1) {
            return 0;
        }
        return static_cast<long long>(y) * 366 + dayOfYear(y, m, d);
    }

    void recordTaskCreated(data::ProductivityState& s) {
        s.xp           += XP_PER_CREATE;
        s.totalCreated += 1;
        if (s.totalCreated == 1) {
            awardIfMissing(s, BADGE_FIRST_TASK);
        }
    }

    void recordTaskEdited(data::ProductivityState& s) {
        s.xp         += XP_PER_EDIT;
        s.totalEdited += 1;
    }

    void recordTaskCompleted(data::ProductivityState& s,
                             const std::string& todayIso) {
        s.xp        += XP_PER_DONE;
        s.totalDone += 1;
        if (!todayIso.empty()) {
            bool already = false;
            for (const std::string& d : s.completionDates) {
                if (d == todayIso) {
                    already = true;
                    break;
                }
            }
            if (!already) {
                s.completionDates.push_back(todayIso);
            }
        }
        if (s.totalDone >= 10) {
            awardIfMissing(s, BADGE_TEN_DONE);
        }
        if (s.totalDone >= 50) {
            awardIfMissing(s, BADGE_FIFTY_DONE);
        }
        int streak = currentStreak(s, todayIso);
        if (streak >= 3) {
            awardIfMissing(s, BADGE_STREAK_3);
        }
        if (streak >= 7) {
            awardIfMissing(s, BADGE_STREAK_7);
        }
    }

    int currentLevel(const data::ProductivityState& s) {
        return 1 + s.xp / XP_PER_LEVEL;
    }

    float levelProgress(const data::ProductivityState& s) {
        int into = s.xp % XP_PER_LEVEL;
        return static_cast<float>(into) / static_cast<float>(XP_PER_LEVEL);
    }

    int currentStreak(const data::ProductivityState& s,
                      const std::string& todayIso) {
        if (s.completionDates.empty() || todayIso.empty()) {
            return 0;
        }
        std::vector<long long> days;
        days.reserve(s.completionDates.size());
        for (const std::string& d : s.completionDates) {
            days.push_back(absoluteDay(d));
        }
        std::sort(days.begin(), days.end());
        days.erase(std::unique(days.begin(), days.end()), days.end());

        long long today = absoluteDay(todayIso);
        int streak = 0;
        long long expected = today;
        for (std::size_t i = days.size(); i > 0; --i) {
            long long d = days[i - 1];
            if (d == expected) {
                streak += 1;
                expected -= 1;
            } else if (d < expected) {
                break;
            }
        }
        return streak;
    }

    std::vector<std::string> allBadgeIds() {
        return {
            BADGE_FIRST_TASK,
            BADGE_TEN_DONE,
            BADGE_FIFTY_DONE,
            BADGE_STREAK_3,
            BADGE_STREAK_7
        };
    }

    const char* badgeLabel(const std::string& id) {
        if (id == BADGE_FIRST_TASK) return "First Task";
        if (id == BADGE_TEN_DONE)   return "Ten Done";
        if (id == BADGE_FIFTY_DONE) return "Fifty Done";
        if (id == BADGE_STREAK_3)   return "Streak x3";
        if (id == BADGE_STREAK_7)   return "Streak x7";
        return id.c_str();
    }

    const char* badgeDescription(const std::string& id) {
        if (id == BADGE_FIRST_TASK) return "Created your first task.";
        if (id == BADGE_TEN_DONE)   return "Completed 10 tasks.";
        if (id == BADGE_FIFTY_DONE) return "Completed 50 tasks.";
        if (id == BADGE_STREAK_3)   return "Three days in a row with completions.";
        if (id == BADGE_STREAK_7)   return "Seven days in a row with completions.";
        return "";
    }

}
