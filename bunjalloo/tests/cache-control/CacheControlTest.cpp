/*
  Copyright (C) 2009 Richard Quirk

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#define private public
#include "CacheControl.h"
#undef private
#include <gtest/gtest.h>

TEST(CacheControl, max_age)
{
  CacheControl cc;
  cc.setCacheControl("max-age=0");
  cc.setDate(0);
  cc.setRequestTime(0);
  cc.setResponseTime(1);

  EXPECT_EQ(0, cc.m_maxAge);
  EXPECT_FALSE(cc.shouldCache(2));

  cc.setCacheControl("max-age=3600");
  cc.setRequestTime(0);
  cc.setResponseTime(1);
  EXPECT_EQ(3600, cc.m_maxAge);
  EXPECT_TRUE(cc.shouldCache(70));

  EXPECT_FALSE(cc.shouldCache(3760));
}

TEST(CacheControl, bad_max_age)
{
  CacheControl cc;
  cc.setCacheControl("max-age=ch");
  cc.setDate(0);
  cc.setRequestTime(0);
  cc.setResponseTime(1);

  EXPECT_EQ(0, cc.m_maxAge);
  EXPECT_FALSE(cc.shouldCache(1));
}

TEST(CacheControl, no_cache)
{
  CacheControl cc;
  cc.setCacheControl("no-cache");
  EXPECT_FALSE(cc.shouldCache(0));

  cc.setCacheControl("max-age=3600");
  cc.setRequestTime(0);
  cc.setResponseTime(0);

  EXPECT_TRUE(cc.shouldCache(60));
  cc.setCacheControl("no-cache");
  EXPECT_FALSE(cc.shouldCache(1));
}

TEST(CacheControl, public_no_cache)
{
  CacheControl cc;
  cc.setCacheControl("public,max-age=0");
  cc.setDate(0);
  cc.setRequestTime(0);
  cc.setResponseTime(0);
  EXPECT_FALSE(cc.shouldCache(1));
}

TEST(CacheControl, reset)
{
  CacheControl cc;
  cc.setCacheControl("no-cache");
  EXPECT_FALSE(cc.shouldCache(1));
  cc.reset();
  cc.setResponseTime(1246821088);
  EXPECT_TRUE(cc.shouldCache(1246821088));
}

TEST(CacheControl, test_expires)
{
  CacheControl cc;
  time_t expire, date;
  cc.setExpires(0);
  cc.setDate(0);
  EXPECT_FALSE(cc.shouldCache(1));

  cc.reset();
  expire = 1246817488; // 5/7 19:11:28
  date = 1246731088; // 4/7 19:11:28
  cc.setRequestTime(1246731088);
  cc.setResponseTime(1246731088);
  cc.setExpires(expire);
  cc.setDate(date);
  EXPECT_TRUE(cc.shouldCache(1246731088));
}
