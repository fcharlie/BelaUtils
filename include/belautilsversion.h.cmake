// Generate code by cmake, don't modify
#ifndef BELAUTILS_VERSION_H
#define BELAUTILS_VERSION_H

#define BELAUTILS_VERSION_MAJOR ${BELAUTILS_VERSION_MAJOR}
#define BELAUTILS_VERSION_MINOR ${BELAUTILS_VERSION_MINOR}
#define BELAUTILS_VERSION_PATCH ${BELAUTILS_VERSION_PATCH}
#define BELAUTILS_BUILD_NUMBER 0

#define BELAUTILS_VERSION L"${BELAUTILS_VERSION_MAJOR}.${BELAUTILS_VERSION_MINOR}.${BELAUTILS_VERSION_PATCH}"
#define BELAUTILS_REVISION L"${BELAUTILS_REVISION}"
#define BELAUTILS_REFNAME L"${BELAUTILS_REFNAME}"
#define BELAUTILS_BUILD_TIME L"${BELAUTILS_BUILD_TIME}"
#define BELAUTILS_REMOTE_URL L"${BELAUTILS_REMOTE_URL}"

#define BELAUTILS_APPLINK                                                                              \
  L"For more information about this tool. \nVisit: <a "                                            \
  L"href=\"https://github.com/fcharlie/belautils\">BelaUtils</"                                               \
  L"a>\nVisit: <a "                                                                                \
  L"href=\"https://forcemz.net/\">forcemz.net</a>"

#define BELAUTILS_APPLINKE                                                                             \
  L"Ask for help with this issue. \nVisit: <a "                                                    \
  L"href=\"https://github.com/fcharlie/belautils/issues\">BelaUtils "                                         \
  L"Issues</a>"

#define BELAUTILS_APPVERSION                                                                           \
  L"Version: ${BELAUTILS_VERSION_MAJOR}.${BELAUTILS_VERSION_MINOR}.${BELAUTILS_VERSION_PATCH}\nCopyright "     \
  L"\xA9 ${BELAUTILS_COPYRIGHT_YEAR}, BelaUtils contributors."

#define BELAUTILSUI_COPYRIGHT L"Copyright \xA9 ${BELAUTILS_COPYRIGHT_YEAR}, BelaUtils contributors."

#endif