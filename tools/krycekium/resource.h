///
#ifndef KRYCEKIUM_RESOURCE_H
#define KRYCEKIUM_RESOURCE_H

#ifdef APPVEYOR_BUILD_NUMBER
#define KRYCEKIUM_BUILD_NUMBER APPVEYOR_BUILD_NUMBER
#else
#define KRYCEKIUM_BUILD_NUMBER 1
#endif

#define TOSTR_1(x) L#x
#define TOSTR(x) TOSTR_1(x)

#define PRIVSUBVER TOSTR(KRYCEKIUM_BUILD_NUMBER)

#define KRYCEKIUM_MAJOR_VERSION 2
#define KRYCEKIUM_MINOR_VERSION 0
#define KRYCEKIUM_PATCH_VERSION 0

#define PRIV_MAJOR TOSTR(KRYCEKIUM_MAJOR_VERSION)
#define PRIV_MINOR TOSTR(KRYCEKIUM_MINOR_VERSION)
#define PRIV_PATCH TOSTR(KRYCEKIUM_PATCH_VERSION)

#define PRIV_VERSION_MAIN PRIV_MAJOR L"." PRIV_MINOR
#define PRIV_VERSION_FULL PRIV_VERSION_MAIN L"." PRIV_PATCH

#ifdef APPVEYOR_BUILD_NUMBER
#define KRYCEKIUM_BUILD_VERSION PRIV_VERSION_FULL L"." PRIVSUBVER L" (appveyor)"
#else
#define KRYCEKIUM_BUILD_VERSION PRIV_VERSION_FULL L"." PRIVSUBVER L" (dev)"
#endif

#define KRYCEKIUM_APPLINK                                                       \
  L"For more information about this tool. \nVisit: <a "                        \
  L"href=\"https://github.com/fcharlie/BelaUtils\">Krycekium</a>\nVisit: <a "      \
  L"href=\"https://forcemz.net/\">forcemz.net</a>"

#define IDI_KRYCEKIUM_ICON 1001
#define COPYRIGHT L"Copyright \xA9 2019 Force Charlie, All Rights Reserved."

#endif
