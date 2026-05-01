#pragma once

#include "libraries.h"
#include "data/types.h"

namespace logic {

    data::Date  today();
    data::Date  zeroDate();
    bool        isDateZero(const data::Date& d);
    bool        isDateValid(const data::Date& d);
    int         compareDates(const data::Date& a, const data::Date& b);
    int         daysBetween(const data::Date& a, const data::Date& b);
    std::string formatDate(const data::Date& d);
    bool        parseDate(const std::string& text, data::Date& out);

}
