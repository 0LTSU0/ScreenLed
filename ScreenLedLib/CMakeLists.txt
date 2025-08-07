project(ScreenLedLib)

# needed so that we can have the class as QObject for easy threading
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

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

target_link_libraries(ScreenLedLib PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_include_directories(ScreenLedLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty)
target_include_directories(ScreenLedLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
