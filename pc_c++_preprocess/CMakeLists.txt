cmake_minimum_required(VERSION 3.10)
project(CPP_preprocess_player)

set(CMAKE_CXX_STANDARD 23)

set(VLC_INSTALL_DIR "C:/Program Files/VideoLAN/VLC" CACHE PATH "Path to VLC installation dir")
set(VLC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vlc")
include_directories(${VLC_DIR}/include)
link_directories(${VLC_DIR})

# Copy vlc dll and its dependencies from VLC_INSTALL_DIR to build dir
if(NOT EXISTS "${VLC_INSTALL_DIR}")
	message(FATAL_ERROR "VLC_INSTALL_DIR ${VLC_INSTALL_DIR} Does not exist")
else()
	file(COPY "${VLC_INSTALL_DIR}/libvlc.dll" DESTINATION "${CMAKE_BINARY_DIR}")
	file(COPY "${VLC_INSTALL_DIR}/libvlccore.dll" DESTINATION "${CMAKE_BINARY_DIR}")
	file(COPY "${VLC_INSTALL_DIR}/plugins" DESTINATION "${CMAKE_BINARY_DIR}")
endif()

add_executable(CPP_preprocess_player main.cpp)

target_link_libraries(CPP_preprocess_player ${VLC_DIR}/lib/libvlc.lib)