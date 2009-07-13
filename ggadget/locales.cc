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

#include "locales.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include "common.h"
#include "logger.h"
#include "string_utils.h"
#include "system_utils.h"

namespace ggadget {

// Stores locale names, short names and Windows locale identifiers.
struct LocaleNameAndId {
  const char *name;
  const char *short_name;
  short windows_locale_id;
};

// The locale names, short_names and Windows locale identifiers mapping table.
// Source for listing is:
//   http://msdn2.microsoft.com/en-us/library/ms776260.aspx
// List below is sorted in ALPHABETICAL ORDER by locale name for use in
// binary search.
// Note: We don't support scripts in locale names, so for locale names with
// script names (such as az_Cyrl_AZ), we remove the extra script part from
// the locale name if IANA defines Suppress-Script for this language or there
// is only one script for that language, otherwise we remove the whole locales.
// IANA source: http://www.iana.org/assignments/language-subtag-registry
static const LocaleNameAndId kLocaleNames[] = {
  {"af-ZA", "af", 0x0436},
  {"am-ET", "am", 0x045e},
  {"ar-AE", NULL, 0x3801},
  {"ar-BH", NULL, 0x3c01},
  {"ar-DZ", NULL, 0x1401},
  {"ar-EG", NULL, 0x0c01},
  {"ar-IQ", NULL, 0x0801},
  {"ar-JO", NULL, 0x2c01},
  {"ar-KW", NULL, 0x3401},
  {"ar-LB", NULL, 0x3001},
  {"ar-LY", NULL, 0x1001},
  {"ar-MA", NULL, 0x1801},
  {"ar-OM", NULL, 0x2001},
  {"ar-QA", NULL, 0x4001},
  {"ar-SA", NULL, 0x0401},
  {"ar-SY", NULL, 0x2801},
  {"ar-TN", NULL, 0x1c01},
  {"ar-YE", NULL, 0x2401},
  {"arn-CL", "arn", 0x047a},
  {"as-IN", "as", 0x044d},
  // No IANA Supress-Script for az.
  // {"az-Cyrl-AZ", NULL, 0x082c},
  // {"az-Latn-AZ", NULL, 0x042c},
  {"ba-RU", "ba", 0x046d},
  {"be-BY", "be", 0x0423},
  {"bg-BG", "bg", 0x0402},
  {"bn-IN", "bn", 0x0445},
  {"bo-CN", "bo", 0x0451},
  {"br-FR", "br", 0x047e},
  {"bs-BA", "bs", 0x141a}, // IANA defines Supress-Script for bs as Latn.
  // {"bs-Cyrl-BA", NULL, 0x201a},
  // {"bs-Latn-BA", NULL, 0x141a},
  {"ca-ES", "ca", 0x0403},
  {"cs-CZ", "cs", 0x0405},
  {"cy-GB", "cy", 0x0452},
  {"da-DK", "da", 0x0406},
  {"de-AT", NULL, 0x0c07},
  {"de-CH", NULL, 0x0807},
  {"de-DE", "de", 0x0407},
  {"de-LI", NULL, 0x1407},
  {"de-LU", NULL, 0x1007},
  {"dsb-DE", "dsb", 0x082e},
  {"dv-MV", "dv", 0x0465},
  {"el-GR", "el", 0x0408},
  {"en-029", NULL, 0x2409},
  {"en-AU", NULL, 0x0c09},
  {"en-BZ", NULL, 0x2809},
  {"en-CA", NULL, 0x1009},
  {"en-GB", NULL, 0x0809},
  {"en-IE", NULL, 0x1809},
  {"en-IN", NULL, 0x4009},
  {"en-JM", NULL, 0x2009},
  {"en-MY", NULL, 0x4409},
  {"en-NZ", NULL, 0x1409},
  {"en-PH", NULL, 0x3409},
  {"en-SG", NULL, 0x4809},
  {"en-TT", NULL, 0x2c09},
  {"en-US", "en", 0x0409},
  {"en-ZA", NULL, 0x1c09},
  {"en-ZW", NULL, 0x3009},
  {"es-AR", NULL, 0x2c0a},
  {"es-BO", NULL, 0x400a},
  {"es-CL", NULL, 0x340a},
  {"es-CO", NULL, 0x240a},
  {"es-CR", NULL, 0x140a},
  {"es-DO", NULL, 0x1c0a},
  {"es-EC", NULL, 0x300a},
  {"es-ES", "es", 0x0c0a},
  // {"es-ES-tradnl", NULL, 0x040a},
  {"es-GT", NULL, 0x100a},
  {"es-HN", NULL, 0x480a},
  {"es-MX", NULL, 0x080a},
  {"es-NI", NULL, 0x4c0a},
  {"es-PA", NULL, 0x180a},
  {"es-PE", NULL, 0x280a},
  {"es-PR", NULL, 0x500a},
  {"es-PY", NULL, 0x3c0a},
  {"es-SV", NULL, 0x440a},
  {"es-US", NULL, 0x540a},
  {"es-UY", NULL, 0x380a},
  {"es-VE", NULL, 0x200a},
  {"et-EE", "et", 0x0425},
  {"eu-ES", "eu", 0x042d},
  {"fa-IR", "fa", 0x0429},
  {"fi-FI", "fi", 0x040b},
  {"fil-PH", "fil", 0x0464},
  {"fo-FO", "fo", 0x0438},
  {"fr-BE", NULL, 0x080c},
  {"fr-CA", NULL, 0x0c0c},
  {"fr-CH", NULL, 0x100c},
  {"fr-FR", "fr", 0x040c},
  {"fr-LU", NULL, 0x140c},
  {"fr-MC", NULL, 0x180c},
  {"fy-NL", NULL, 0x0462},
  {"ga-IE", "ga", 0x083c},
  {"gbz-AF", "gbz", 0x048c},
  {"gl-ES", "gl", 0x0456},
  {"gsw-FR", "gsw", 0x0484},
  {"gu-IN", "gu", 0x0447},
  // No IANA Supress-Script for ha.
  // {"ha-Latn-NG", NULL, 0x0468},
  {"he-IL", "he", 0x040d},
  {"hi-IN", "hi", 0x0439},
  {"hr-BA", NULL, 0x101a},
  {"hr-HR", "hr", 0x041a},
  {"hu-HU", "hu", 0x040e},
  {"hy-AM", "hy", 0x042b},
  {"id-ID", "id", 0x0421},
  {"ig-NG", "ig", 0x0470},
  {"ii-CN", "ii", 0x0478},
  {"is-IS", "is", 0x040f},
  {"it-CH", NULL, 0x0810},
  {"it-IT", "it", 0x0410},
  // No IANA Supress-Script for iu.
  // {"iu-Cans-CA", NULL, 0x045d},
  // {"iu-Latn-CA", NULL, 0x085d},
  {"ja-JP", "ja", 0x0411},
  {"ka-GE", "ka", 0x0437},
  {"kh-KH", "kh", 0x0453},
  {"kk-KZ", "kk", 0x043f},
  {"kl-GL", "kl", 0x046f},
  {"kn-IN", "kn", 0x044b},
  {"ko-KR", "ko", 0x0412},
  {"kok-IN", "kok", 0x0457},
  {"ky-KG", "ky", 0x0440},
  {"lb-LU", "lb", 0x046e},
  {"lo-LA", "lo", 0x0454},
  {"lt-LT", "lt", 0x0427},
  {"lv-LV", "lv", 0x0426},
  {"mi-NZ", "mi", 0x0481},
  {"mk-MK", "mk", 0x042f},
  {"ml-IN", "ml", 0x044c},
  // No IANA Supress-Script for mn.
  // {"mn-Cyrl-MN", NULL, 0x0450},
  // {"mn-Mong-CN", NULL, 0x0850},
  {"moh-CA", "moh", 0x047c},
  {"mr-IN", "mr", 0x044e},
  {"ms-BN", NULL, 0x083e},
  {"ms-MY", "ms", 0x043e},
  {"mt-MT", "mt", 0x043a},
  {"nb-NO", "nb", 0x0414},
  {"ne-NP", "ne", 0x0461},
  {"nl-BE", NULL, 0x0813},
  {"nl-NL", "nl", 0x0413},
  {"nn-NO", "nn", 0x0814},
  {"no-NO", "no", 0x0414},
  {"ns-ZA", "ns", 0x046c},
  {"oc-FR", "oc", 0x0482},
  {"or-IN", "or", 0x0448},
  {"pa-IN", "pa", 0x0446},
  {"pl-PL", "pl", 0x0415},
  {"ps-AF", "ps", 0x0463},
  // No short name for pt-BR and pt-PT.
  {"pt-BR", NULL, 0x0416},
  {"pt-PT", NULL, 0x0816},
  {"qut-GT", "qut", 0x0486},
  {"quz-BO", NULL, 0x046b},
  {"quz-EC", NULL, 0x086b},
  {"quz-PE", NULL, 0x0c6b},
  {"rm-CH", "rm", 0x0417},
  {"ro-RO", "ro", 0x0418},
  {"ru-RU", "ru", 0x0419},
  {"rw-RW", "rw", 0x0487},
  {"sa-IN", "sa", 0x044f},
  {"sah-RU", "sah", 0x0485},
  {"se-FI", NULL, 0x0c3b},
  {"se-NO", NULL, 0x043b},
  {"se-SE", "se", 0x083b},
  {"si-LK", "si", 0x045b},
  {"sk-SK", "sk", 0x041b},
  {"sl-SI", "sl", 0x0424},
  {"sma-NO", NULL, 0x183b},
  {"sma-SE", NULL, 0x1c3b},
  {"smj-NO", NULL, 0x103b},
  {"smj-SE", NULL, 0x143b},
  {"smn-FI", "smn", 0x243b},
  {"sms-FI", "sms", 0x203b},
  {"sq-AL", "sq", 0x041c},
  // No IANA Supress-Script for sr.
  // {"sr-Cyrl-BA", NULL, 0x1c1a},
  // {"sr-Cyrl-CS", NULL, 0x0c1a},
  // {"sr-Latn-BA", NULL, 0x181a},
  // {"sr-Latn-CS", NULL, 0x081a},
  {"sv-FI", NULL, 0x081d},
  {"sv-SE", "sv", 0x041d},
  {"sw-KE", "sw", 0x0441},
  {"syr-SY", "syr", 0x045a},
  {"ta-IN", "ta", 0x0449},
  {"te-IN", "te", 0x044a},
  // No IANA Supress-Script for tg.
  // {"tg-Cyrl-TJ", NULL, 0x0428},
  {"th-TH", "th", 0x041e},
  {"tk-TM", "tk", 0x0442},
  // No IANA Supress-Script for tmz.
  // {"tmz-Latn-DZ", NULL, 0x085f},
  {"tn-ZA", "tn", 0x0432},
  {"tr-IN", NULL, 0x0820},
  {"tr-TR", "tr", 0x041f},
  {"tt-RU", "tt", 0x0444},
  {"ug-CN", "ug", 0x0480},
  {"uk-UA", "uk", 0x0422},
  {"ur-PK", "ur", 0x0420},
  // No IANA Supress-Script for uz
  // {"uz-Cyrl-UZ", NULL, 0x0843},
  // {"uz-Latn-UZ", NULL, 0x0443},
  {"vi-VN", "vi", 0x042a},
  {"wen-DE", "wen", 0x042e},
  {"wo-SN", "wo", 0x0488},
  {"xh-ZA", "xh", 0x0434},
  {"yo-NG", "yo", 0x046a},
  {"za-CN", "za", 0},
  // No default short name for zh-* locales.
  {"zh-CN", NULL, 0x0804},
  {"zh-HK", NULL, 0x0c04},
  {"zh-MO", NULL, 0x1404},
  {"zh-SG", NULL, 0x1004},
  {"zh-TW", NULL, 0x0404},
  {"zu-ZA", "zu", 0x0435}
};

static inline bool LocaleNameCompare(const LocaleNameAndId &v1,
                                     const LocaleNameAndId &v2) {
  return strcmp(v1.name, v2.name) < 0;
}

static bool WindowsLocaleIDToString(short windows_locale_id,
                                    std::string *result) {
  char buffer[12];
  if (windows_locale_id > 0) {
    snprintf(buffer, sizeof(buffer), "%d", windows_locale_id);
    result->assign(buffer);
    return true;
  }
  return false;
}

bool GetLocaleWindowsIDString(const char *name, std::string *windows_id) {
  ASSERT(windows_id);
  LocaleNameAndId key = { name, NULL, 0 };
  const LocaleNameAndId *pos = std::lower_bound(
      kLocaleNames, kLocaleNames + arraysize(kLocaleNames), key,
      LocaleNameCompare);

  ASSERT(pos);
  if (strcmp(name, pos->name) == 0)
    return WindowsLocaleIDToString(pos->windows_locale_id, windows_id);

  // The given name may be in short form.
  size_t len = strlen(name);
  for (; pos < kLocaleNames + arraysize(kLocaleNames); ++pos) {
    if (strncmp(name, pos->name, len) == 0) {
      if (pos->short_name && strcmp(name, pos->short_name) == 0)
        return WindowsLocaleIDToString(pos->windows_locale_id, windows_id);
    }
  }
  return false;
}

bool GetLocaleShortName(const char *name, std::string *short_name) {
  ASSERT(short_name);
  LocaleNameAndId key = { name, NULL, 0 };
  const LocaleNameAndId *pos = std::lower_bound(
      kLocaleNames, kLocaleNames + arraysize(kLocaleNames), key,
      LocaleNameCompare);

  ASSERT(pos);
  if (strcmp(name, pos->name) == 0 && pos->short_name) {
    // Found the name ==> short_name mapping.
    *short_name = pos->short_name;
    return true;
  }

  size_t len = strlen(name);
  for (; pos < kLocaleNames + arraysize(kLocaleNames); ++pos) {
    if (strncmp(name, pos->name, len) == 0) {
      if (pos->short_name && strcmp(name, pos->short_name) == 0) {
        // The input name is already a short name.
        *short_name = name;
        return true;
      }
    } else {
      break;
    }
  }
  return false;
}

std::string GetSystemLocaleName() {
  std::string language, territory;
  if (GetSystemLocaleInfo(&language, &territory)) {
    if (territory.length()) {
      std::string full_locale(ToLower(language));
      full_locale.append("-");
      full_locale.append(ToUpper(territory));
      std::string short_locale;
      // To keep compatible with the Windows version, we must return the short
      // name for many locales.
      return GetLocaleShortName(full_locale.c_str(), &short_locale) ?
             short_locale : full_locale;
    }
    return language;
  }
  return "en";
}

} // namespace ggadget
