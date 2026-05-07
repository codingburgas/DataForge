#pragma once

#include "libraries.h"
#include "data/types.h"

namespace logic {

    // XP awards. Tuned so casual use ticks up quickly without trivialising
    // mark-as-done, which is the action we most want to reinforce.
    constexpr int XP_PER_CREATE = 10;
    constexpr int XP_PER_EDIT   = 5;
    constexpr int XP_PER_DONE   = 50;
    constexpr int XP_PER_LEVEL  = 100;

    void recordTaskCreated(data::ProductivityState& s);
    void recordTaskEdited (data::ProductivityState& s);
    void recordTaskCompleted(data::ProductivityState& s,
                             const std::string& todayIso);

    // Level derived from total XP. Level 1 starts at 0 XP.
    int currentLevel(const data::ProductivityState& s);

    // Progress toward next level in [0,1].
    float levelProgress(const data::ProductivityState& s);

    // Consecutive days ending today with at least one completion.
    int currentStreak(const data::ProductivityState& s,
                      const std::string& todayIso);

    // All badge identifiers the system can award.
    std::vector<std::string> allBadgeIds();

    // Localised label & description for a badge id.
    const char* badgeLabel(const std::string& id);
    const char* badgeDescription(const std::string& id);

    bool hasBadge(const data::ProductivityState& s, const std::string& id);

}
