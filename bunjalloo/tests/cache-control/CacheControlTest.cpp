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
  cc.setSeconds(1);

  EXPECT_EQ(0U, cc.m_maxAge);
  EXPECT_FALSE(cc.shouldCache());

  cc.setCacheControl("max-age=3600");
  cc.setSeconds(60);
  EXPECT_EQ(3600U, cc.m_maxAge);
  EXPECT_TRUE(cc.shouldCache());

  cc.setSeconds(3760);
  EXPECT_FALSE(cc.shouldCache());
}

TEST(CacheControl, bad_max_age)
{
  CacheControl cc;
  cc.setCacheControl("max-age=ch");
  cc.setSeconds(1);

  EXPECT_EQ(0U, cc.m_maxAge);
  EXPECT_FALSE(cc.shouldCache());
}

TEST(CacheControl, no_cache)
{
  CacheControl cc;
  cc.setCacheControl("no-cache");
  EXPECT_FALSE(cc.shouldCache());

  cc.setCacheControl("max-age=3600");
  cc.setSeconds(60);

  EXPECT_TRUE(cc.shouldCache());
  cc.setCacheControl("no-cache");
  EXPECT_FALSE(cc.shouldCache());
}

TEST(CacheControl, public_no_cache)
{
  CacheControl cc;
  cc.setCacheControl("public,max-age=0");
  cc.setSeconds(1);
  EXPECT_FALSE(cc.shouldCache());
}

TEST(CacheControl, reset)
{
  CacheControl cc;
  cc.setCacheControl("no-cache");
  EXPECT_FALSE(cc.shouldCache());
  cc.reset();
  EXPECT_TRUE(cc.shouldCache());
}
