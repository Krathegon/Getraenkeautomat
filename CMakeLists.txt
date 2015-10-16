cmake_minimum_required (VERSION 2.8)
project (Getraenkeautomat)

#file(GLOB_RECURSE SOURCE_TMP "*.c" )
#list(APPEND SRC ${SOURCE_TMP})

#list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules)
#find_package(GLIB REQUIRED)

include_directories (
	#"${PROJECT_SOURCE_DIR}/wiringPi"
	"${PROJECT_SOURCE_DIR}/nxprdlib/intfs"
    "${PROJECT_SOURCE_DIR}/nxprdlib/types"
    #"${PROJECT_SOURCE_DIR}/nxprdlib/comps/phpalSli15693/src/Sw"
	"/usr/include/glib-2.0"
	"/usr/lib/arm-linux-gnueabihf/glib-2.0/include"
	"/usr/lib/x86_64-linux-gnu/glib-2.0/include"
)


set(EXT_LIBS  pthread  wiringPi  glib-2.0  m)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_definitions(-g -O0)

list(APPEND SOURCE "${PROJECT_SOURCE_DIR}/main.cpp")
list(APPEND SOURCE "${PROJECT_SOURCE_DIR}/protocol.cpp")
list(APPEND SOURCE "${PROJECT_SOURCE_DIR}/NXP.cpp")
list(APPEND SOURCE "${PROJECT_SOURCE_DIR}/SerialDevice.cpp")
list(APPEND SOURCE "${PROJECT_SOURCE_DIR}/CWBoard.cpp")
list(APPEND SOURCE "${PROJECT_SOURCE_DIR}/RFIDBoard.cpp")
list(APPEND SOURCE "${PROJECT_SOURCE_DIR}/Card.cpp")
source_group(src "${PROJECT_SOURCE_DIR}/main.cpp")

file(GLOB_RECURSE SOURCE_TMP "${PROJECT_SOURCE_DIR}/nxprdlib/*.c" )
list(APPEND SOURCE ${SOURCE_TMP})
#source_group(wiringPi FILES ${SOURCE_TMP})

#list(REMOVE_ITEM SOURCE "${PROJECT_SOURCE_DIR}/nxprdlib/ex/NxpRdLib.c")


add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} ${EXT_LIBS})
#set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
#set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
