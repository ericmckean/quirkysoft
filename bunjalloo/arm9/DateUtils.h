#ifndef DateUtils_h_seen
#define DateUtils_h_seen

#include <time.h>
#include <string>

class DateUtils
{
  public:
    static time_t parseDate(const char *date);
    static std::string formatTime(time_t t);
};
#endif
