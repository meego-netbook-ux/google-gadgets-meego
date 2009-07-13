/*
  Copyright 2008 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include "ggadget/locales.h"
#include "unittest/gtest.h"

using namespace ggadget;

TEST(Locales, GetLocaleWindowsIDString) {
  std::string windows_id;
  ASSERT_TRUE(GetLocaleWindowsIDString("zh-CN", &windows_id));
  ASSERT_EQ(std::string("2052"), windows_id);
  ASSERT_TRUE(GetLocaleWindowsIDString("en-US", &windows_id));
  ASSERT_EQ(std::string("1033"), windows_id);
  ASSERT_TRUE(GetLocaleWindowsIDString("en", &windows_id));
  ASSERT_EQ(std::string("1033"), windows_id);
  ASSERT_FALSE(GetLocaleWindowsIDString("zh", &windows_id));
}

TEST(Locales, GetLocaleShortName) {
  std::string short_name;
  // zh-CN has no short form.
  ASSERT_FALSE(GetLocaleShortName("zh-CN", &short_name));
  ASSERT_TRUE(GetLocaleShortName("en-US", &short_name));
  ASSERT_EQ(std::string("en"), short_name);
  ASSERT_TRUE(GetLocaleShortName("en", &short_name));
  ASSERT_EQ(std::string("en"), short_name);
  // zh is not an accepted short form.
  ASSERT_FALSE(GetLocaleShortName("zh", &short_name));
}

TEST(Locales, GetSystemLocaleName) {
  setlocale(LC_MESSAGES, "en_US.UTF-8");
  ASSERT_EQ(std::string("en"), GetSystemLocaleName());
  setlocale(LC_MESSAGES, "en_US");
  ASSERT_EQ(std::string("en"), GetSystemLocaleName());
  setlocale(LC_MESSAGES, "zh_CN.UTF-8");
  ASSERT_EQ(std::string("zh-CN"), GetSystemLocaleName());
  setlocale(LC_MESSAGES, "C");
  ASSERT_EQ(std::string("en"), GetSystemLocaleName());
}

int main(int argc, char **argv) {
  testing::ParseGTestFlags(&argc, argv);
  return RUN_ALL_TESTS();
}
