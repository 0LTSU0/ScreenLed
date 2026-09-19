project(ScreenLedLib)

# needed so that we can have the class as QObject for easy threading
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if (WIN32)
	set(OS_SPECIFIC_SOURCES
		ScreenCapper_Windows.cpp
		ScreenCapper_Windows.h
		Sender_Windows.h
		Sender_Windows.cpp)
else()
	set(OS_SPECIFIC_SOURCES
	ScreenCapper_Linux.cpp
	ScreenCapper_Linux.h
	Sender_Linux.h
	Sender_Linux.cpp)
endif()

set(ALGO_SOURCES
	algomean.h
	algomean.cpp
	algomedian.h
	algomedian.cpp
	algoflashboost.h
	algoflashboost.cpp
)

add_library(ScreenLedLib STATIC
	ScreenLedLib.h
	ScreenLedLib.cpp
	Commons.h
	Mailbox.h
	Analyzer.cpp
	Analyzer.h
	${OS_SPECIFIC_SOURCES}
	${ALGO_SOURCES}
)

target_compile_definitions(ScreenLedLib PRIVATE WIN32_LEAN_AND_MEAN)
set_source_files_properties(ScreenCapper_Linux.cpp PROPERTIES COMPILE_OPTIONS "-mssse3")

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
if (WIN32)
	target_compile_definitions(ScreenLedLib PRIVATE NOMINMAX)
	target_link_libraries(ScreenLedLib PRIVATE ws2_32)
endif()

target_include_directories(ScreenLedLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty)
target_include_directories(ScreenLedLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
