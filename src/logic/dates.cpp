#include "logic/dates.h"
#include "data/date.h"

namespace logic {

    data::Date today() {
        return data::makeToday();
    }

    data::Date zeroDate() {
        return data::makeZeroDate();
    }

    bool isDateZero(const data::Date& d) {
        return data::isDateZero(d);
    }

    bool isDateValid(const data::Date& d) {
        return data::isDateValid(d);
    }

    int compareDates(const data::Date& a, const data::Date& b) {
        return data::compareDates(a, b);
    }

    int daysBetween(const data::Date& a, const data::Date& b) {
        return data::daysBetween(a, b);
    }

    std::string formatDate(const data::Date& d) {
        return data::formatDate(d);
    }

    bool parseDate(const std::string& text, data::Date& out) {
        return data::parseDate(text, out);
    }

}
