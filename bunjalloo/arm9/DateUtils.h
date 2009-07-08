#ifndef DateUtils_h_seen
#define DateUtils_h_seen

#include <time.h>

class DateUtils
{
  public:
    static time_t parseDate(const char *date);
};
#endif
