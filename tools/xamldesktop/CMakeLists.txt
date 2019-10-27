#

#

add_executable(xamldesktop WIN32
  xamldesktop.cc
  xamldesktop.manifest
)

target_link_libraries(xamldesktop
  windowsapp
  ole32
  user32
  shcore
)