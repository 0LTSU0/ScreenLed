project(ScreenLedLib)

if (WIN32)
	set(OS_SPECIFIC_SOURCES ScreenCapWindows.h ScreenCapWindows.cpp)
else()
	set(OS_SPECIFIC_SOURCES ScreenCapLinux.h ScreenCapLinux.cpp)
endif()

add_library(ScreenLedLib
	ScreenCapBase.h
	ScreenCapBase.cpp
	${OS_SPECIFIC_SOURCES}
)

target_include_directories(ScreenLedLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(ScreenLedLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
