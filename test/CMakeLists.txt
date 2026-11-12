cmake_minimum_required(VERSION 3.14)
project(screenled_tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Important for MSVC builds
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(CTest)
include(GoogleTest)

add_executable(screenled_tests
    test_mailbox.cpp
	test_algo_mean.cpp
	test_algo_median.cpp
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_subdirectory(../ScreenLedLib ${CMAKE_CURRENT_BINARY_DIR}/lib)

target_link_libraries(screenled_tests
    PRIVATE
        ScreenLedLib
		Qt${QT_VERSION_MAJOR}::Widgets
        GTest::gtest_main
)

target_include_directories(screenled_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/helpers
)

if(WIN32)
    add_custom_command(TARGET screenled_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Core>
            $<TARGET_FILE_DIR:screenled_tests>
        VERBATIM
    )
endif()
gtest_discover_tests(screenled_tests)
