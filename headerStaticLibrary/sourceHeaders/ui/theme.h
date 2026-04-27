#pragma once

#include "libraries.h"
#include "data/types.h"

namespace ui {

    void applyDarkTheme();
    void applyLightTheme();

    struct UrgencyColor {
        float r;
        float g;
        float b;
        float a;
    };

    UrgencyColor urgencyForDeadline(const data::Date& deadline,
                                    const data::Date& today,
                                    data::Status status);

    UrgencyColor colorForPriority(data::Priority p);

    const char* priorityLabel(data::Priority p);
    const char* statusLabel(data::Status s);

}
