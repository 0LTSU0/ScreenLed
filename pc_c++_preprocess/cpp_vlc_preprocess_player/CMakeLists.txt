cmake_minimum_required(VERSION 3.5)

project(cpp_vlc_preprocess_player VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        preprocessplayer.cpp
        preprocessplayer.h
        preprocessplayer.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(cpp_vlc_preprocess_player
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        #VLCWidget.h
        #VLCWidget.cpp
        vlcplayer.h vlcplayer.cpp
        #UIUpdateWorker.h
        udpworker.h udpworker.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET cpp_vlc_preprocess_player APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(cpp_vlc_preprocess_player SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(cpp_vlc_preprocess_player
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Copy vlc dll and its dependencies from VLC_INSTALL_DIR to build dir and link
set(VLC_VERSION "v4.0.0-dev" CACHE STRING "VLC version from Thirdparty" FORCE)
set(VLC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vlc-${VLC_VERSION}")
include_directories(${VLC_DIR}/include)
link_directories(${VLC_DIR})
if(NOT EXISTS "${VLC_DIR}")
        message(FATAL_ERROR "VLC_DIR ${VLC_DIR} Does not exist")
else()
        file(COPY "${VLC_DIR}/libvlc.dll" DESTINATION "${CMAKE_BINARY_DIR}")
        file(COPY "${VLC_DIR}/libvlccore.dll" DESTINATION "${CMAKE_BINARY_DIR}")
        file(COPY "${VLC_DIR}/plugins" DESTINATION "${CMAKE_BINARY_DIR}")
endif()

target_link_libraries(cpp_vlc_preprocess_player PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${VLC_DIR}/lib/libvlc.lib)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.cpp_vlc_preprocess_player)
endif()
set_target_properties(cpp_vlc_preprocess_player PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS cpp_vlc_preprocess_player
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(VLC_INSTALL_DIR "C:/Program Files/VideoLAN/VLC" CACHE PATH "Path to VLC installation dir")
set(VLC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vlc")
include_directories(${VLC_DIR}/include)
link_directories(${VLC_DIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(cpp_vlc_preprocess_player)
endif()
