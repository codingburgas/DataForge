#include "data/date.h"

namespace data {

    // The "zero date" {0,0,0} is a sentinel meaning "no date set" —
    // used by tasks with no deadline. Real dates always start at year
    // 1900 so there is no ambiguity.
    Date makeZeroDate() {
        return Date{ 0, 0, 0 };
    }

    bool isDateZero(const Date& d) {
        return d.year == 0 && d.month == 0 && d.day == 0;
    }

    // Standard Gregorian leap year rule.
    static bool isLeapYear(int year) {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    static int daysInMonth(int year, int month) {
        static const int TABLE[12] = {
            31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
        };
        if (month < 1 || month > 12) {
            return 0;
        }
        if (month == 2 && isLeapYear(year)) {
            return 29;
        }
        return TABLE[month - 1];
    }

    // The zero date is treated as valid so "no deadline" round-trips
    // through the validator without complaint. Real dates must lie in
    // [1900, 9999] and resolve to a real day-of-month.
    bool isDateValid(const Date& d) {
        if (isDateZero(d)) {
            return true;
        }
        if (d.year < 1900 || d.year > 9999) {
            return false;
        }
        if (d.month < 1 || d.month > 12) {
            return false;
        }
        int dim = daysInMonth(d.year, d.month);
        if (d.day < 1 || d.day > dim) {
            return false;
        }
        return true;
    }

    // Total ordering with one twist: zero dates sort AFTER real dates.
    // That puts "no deadline" tasks at the bottom of an ascending sort,
    // which is what users expect (real deadlines come first, undated
    // tasks fall to the end).
    int compareDates(const Date& a, const Date& b) {
        bool azero = isDateZero(a);
        bool bzero = isDateZero(b);
        if (azero && bzero) {
            return 0;
        }
        if (azero) {
            return 1;
        }
        if (bzero) {
            return -1;
        }
        if (a.year  != b.year)  return a.year  < b.year  ? -1 : 1;
        if (a.month != b.month) return a.month < b.month ? -1 : 1;
        if (a.day   != b.day)   return a.day   < b.day   ? -1 : 1;
        return 0;
    }

    // Local time on purpose — deadlines are calendar concepts, not
    // timestamps, so a user on the train at midnight UTC should see
    // the date their wall clock shows.
    Date makeToday() {
        std::time_t now = std::time(nullptr);
        std::tm local{};
    #if defined(_MSC_VER)
        localtime_s(&local, &now);
    #else
        std::tm* tmp = std::localtime(&now);
        if (tmp != nullptr) {
            local = *tmp;
        }
    #endif
        Date d{};
        d.year  = local.tm_year + 1900;
        d.month = local.tm_mon  + 1;
        d.day   = local.tm_mday;
        return d;
    }

    // ISO 8601 date format (YYYY-MM-DD). Zero dates serialise to an
    // empty string so they take zero bytes in the .dftasks file.
    std::string formatDate(const Date& d) {
        if (isDateZero(d)) {
            return std::string();
        }
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
                      d.year, d.month, d.day);
        return std::string(buf);
    }

    // Strict ISO 8601 parser. Accepts only "YYYY-MM-DD" and the empty
    // string (which round-trips to a zero date). Anything else is
    // rejected so the loader cannot silently accept malformed input.
    bool parseDate(const std::string& text, Date& out) {
        if (text.empty()) {
            out = makeZeroDate();
            return true;
        }
        if (text.size() != 10) {
            return false;
        }
        if (text[4] != '-' || text[7] != '-') {
            return false;
        }
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (i == 4 || i == 7) {
                continue;
            }
            char c = text[i];
            if (c < '0' || c > '9') {
                return false;
            }
        }
        Date tmp{};
        tmp.year  = std::atoi(text.substr(0, 4).c_str());
        tmp.month = std::atoi(text.substr(5, 2).c_str());
        tmp.day   = std::atoi(text.substr(8, 2).c_str());
        if (!isDateValid(tmp)) {
            return false;
        }
        out = tmp;
        return true;
    }

    // Standard Julian Day Number formula (Fliegel & Van Flandern).
    // Converts a calendar date to a single integer so daysBetween() can
    // be a plain subtraction — the formula handles leap years and
    // varying month lengths automatically.
    static long long julianDayNumber(const Date& d) {
        int y = d.year;
        int m = d.month;
        int day = d.day;
        if (m <= 2) {
            y  -= 1;
            m  += 12;
        }
        long long A = y / 100;
        long long B = 2 - A + A / 4;
        long long jdn =
              static_cast<long long>(365.25  * (y + 4716))
            + static_cast<long long>(30.6001 * (m + 1))
            + day + B - 1524;
        return jdn;
    }

    int daysBetween(const Date& a, const Date& b) {
        long long ja = julianDayNumber(a);
        long long jb = julianDayNumber(b);
        return static_cast<int>(jb - ja);
    }

}
