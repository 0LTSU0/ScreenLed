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

if (UNIX)
	# sudo apt install libx11-dev libxext-dev libxrandr-dev
	# sudo apt install libopencv-dev
	find_package(X11 REQUIRED)
	target_include_directories(ScreenLedLib PRIVATE ${X11_INCLUDE_DIR})
	target_link_libraries(ScreenLedLib PRIVATE ${X11_LIBRARIES} Xrandr)
	find_package(OpenCV REQUIRED)
	target_link_libraries(ScreenLedLib PRIVATE ${OpenCV_LIBS})
	target_include_directories(ScreenLedLib PRIVATE ${OpenCV_INCLUDE_DIRS})
endif()

target_link_libraries(ScreenLedLib PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_include_directories(ScreenLedLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty)
target_include_directories(ScreenLedLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
