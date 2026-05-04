#include "logic/dates.h"
#include "data/date.h"

namespace logic {

    // Logic-tier facade over the data-tier Date helpers. The UI is not
    // allowed to talk to the data namespace directly, so every date
    // operation it needs is re-exported here.

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
