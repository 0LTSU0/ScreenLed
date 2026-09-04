cmake_minimum_required(VERSION 3.5)

project(ScreenLedGUI VERSION 0.2.0 LANGUAGES CXX)

# --- Get git hash ---
find_package(Git QUIET)

if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND ${GIT_EXECUTABLE} status --porcelain
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_STATUS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(GIT_STATUS)
        set(GIT_DIRTY "-dirty")
    else()
        set(GIT_DIRTY "")
    endif()
else()
    set(GIT_HASH "nogit")
    set(GIT_DIRTY "")
endif()

set(APP_VERSION "${PROJECT_VERSION}-${GIT_HASH}${GIT_DIRTY}")

# Generate header
configure_file(
    ${CMAKE_SOURCE_DIR}/version.h.in
    ${CMAKE_BINARY_DIR}/generated/version.h
    @ONLY
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        main.cpp
        consts.h
        receiverrunner.h
        receiverrunner.cpp
        receiverrunnerssh.h
        receiverrunnerssh.cpp
        maingui.h maingui.cpp maingui.ui
        aboutwindow.h aboutwindow.cpp aboutwindow.ui
        settingswindow.h settingswindow.cpp settingswindow.ui
        receiverconsole.h receiverconsole.cpp receiverconsole.ui
        receiverrunnerssh_statuslistener.h receiverrunnerssh_statuslistener.cpp
        errordialog.h errordialog.cpp errordialog.ui
        configurator.h configurator.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ScreenLedGUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        qtresources.qrc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ScreenLedGUI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ScreenLedGUI SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ScreenLedGUI
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ScreenLedGUI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(ScreenLedGUI PRIVATE Qt${QT_VERSION_MAJOR}::Network)

# Add ScreenLed library and json header
add_subdirectory(../ScreenLedLib ${CMAKE_CURRENT_BINARY_DIR}/lib)
target_link_libraries(ScreenLedGUI PRIVATE ScreenLedLib)
target_include_directories(ScreenLedGUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty)
target_include_directories(ScreenLedGUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ScreenLedLib)
target_include_directories(ScreenLedGUI PRIVATE ${CMAKE_BINARY_DIR}/generated)

# Add libssh
add_subdirectory(../ThirdParty/libssh2/ ${CMAKE_CURRENT_BINARY_DIR}/libssh2)
target_link_libraries(ScreenLedGUI PRIVATE libssh2_static)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ScreenLedGUI)
endif()
set_target_properties(ScreenLedGUI PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ScreenLedGUI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ScreenLedGUI)
endif()
