set(JSON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/json/src PARENT_SCOPE)
set(NXP_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/nxprdlib/intfs" "${CMAKE_CURRENT_SOURCE_DIR}/nxprdlib/types" PARENT_SCOPE)
set(GLIB_INCLUDE_DIRS "/usr/include/glib-2.0" "/usr/lib/arm-linux-gnueabihf/glib-2.0/include" PARENT_SCOPE)
#for desktop x64 pc use "/usr/lib/x86_64-linux-gnu/glib-2.0/include"

set(BUILD_CPR_TESTS OFF CACHE BOOL "Don't build cpr tests")
find_package(CURL)
if(CURL_FOUND)
    set(USE_SYSTEM_CURL ON CACHE BOOL "Use the system curl for faster builds")
endif()

add_subdirectory(cpr)