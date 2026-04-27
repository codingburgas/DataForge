#pragma once

#include "libraries.h"
#include "data/types.h"

namespace data {

    bool        isDateZero(const Date& d);
    bool        isDateValid(const Date& d);
    int         compareDates(const Date& a, const Date& b);
    Date        makeToday();
    std::string formatDate(const Date& d);
    bool        parseDate(const std::string& text, Date& out);
    int         daysBetween(const Date& a, const Date& b);
    Date        makeZeroDate();

}
