# This CmakeLists is only relevant when building this as dll for use with ScreenLedGUI. When building the basic console application, simply use the vs solution

project(ScreenLedLib)

add_library(ScreenLedLib SHARED
	analyzer.cpp
	analyzer.h
	configurator.cpp
	configurator.h
	main.cpp
)


# include boost
set(BOOST_ROOT D:/boost_1_83_0)
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.83.0 REQUIRED) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})  
    target_link_libraries(ScreenLedLib ${Boost_LIBRARIES})
endif()

target_include_directories(ScreenLedLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


