#include <gtest/gtest.h>
#include "DateUtils.h"

TEST(DateUtils, parse_date)
{
  /*
     from time import strftime, strptime
     strftime('%s', strptime("Sun, 05 Jul 2009 19:11:28 GMT", "%a, %d %b %Y %H:%M:%S %Z"))
  */
  time_t expected(1246817488);
  time_t result = DateUtils::parseDate("Sun, 05 Jul 2009 19:11:28 GMT");

  EXPECT_EQ(expected, result);
}

TEST(DateUtils, zero_time)
{
  time_t expected(0);
  time_t result = DateUtils::parseDate("Thu, 01 Jan 1970 00:00:00 GMT");
  EXPECT_EQ(expected, result);
}


TEST(DateUtils, parse_bogus)
{
  /*
     from time import strftime, strptime
     strftime('%s', strptime("Sun, 05 Jul 2009 19:11:28 GMT", "%a, %d %b %Y %H:%M:%S %Z"))
  */
  time_t expected(0);
  time_t result = DateUtils::parseDate("KKKKK");

  EXPECT_EQ(expected, result);
}


