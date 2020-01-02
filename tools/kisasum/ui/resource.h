///
#ifndef KISASUM_RESOURCE_H
#define KISASUM_RESOURCE_H

#ifdef APPVEYOR_BUILD_NUMBER
#define KISASUM_BUILD_NUMBER APPVEYOR_BUILD_NUMBER
#else
#define KISASUM_BUILD_NUMBER 1
#endif

#define TOSTR_1(x) L#x
#define TOSTR(x) TOSTR_1(x)

#define PRIVSUBVER TOSTR(KISASUM_BUILD_NUMBER)

#define KISASUM_MAJOR_VERSION 2
#define KISASUM_MINOR_VERSION 0
#define KISASUM_PATCH_VERSION 0

#define PRIV_MAJOR TOSTR(KISASUM_MAJOR_VERSION)
#define PRIV_MINOR TOSTR(KISASUM_MINOR_VERSION)
#define PRIV_PATCH TOSTR(KISASUM_PATCH_VERSION)

#define PRIV_VERSION_MAIN PRIV_MAJOR L"." PRIV_MINOR
#define PRIV_VERSION_FULL PRIV_VERSION_MAIN L"." PRIV_PATCH

#ifdef APPVEYOR_BUILD_NUMBER
#define KISASUM_BUILD_VERSION PRIV_VERSION_FULL L"." PRIVSUBVER L" (appveyor)"
#else
#define KISASUM_BUILD_VERSION PRIV_VERSION_FULL L"." PRIVSUBVER L" (dev)"
#endif

#define KISASUM_APPLINK                                                        \
  L"For more information about this tool. \nVisit: <a "                        \
  L"href=\"https://github.com/fcharlie/BelaUtils\">Kisasum</a>\nVisit: <a "    \
  L"href=\"https://forcemz.net/\">forcemz.net</a>"

#define COPYRIGHT L"Copyright \xA9 2020 Force Charlie, All Rights Reserved."
#define FULL_COPYRIGHT                                                         \
  L"Prerelease: 2.0\nCopyright \xA9 2020, Force Charlie. All Rights Reserved."

// ICON
#define IDI_KISASUM_ICON 500

// about

#endif
