cmake_minimum_required(VERSION 2.8)

project(asio_proto)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
	set(Boost_USE_STATIC_LIBS ON)
endif(WIN32)

find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${BOOST_LIBRARYDIR})

#### mats proto
set(MATS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/mats)
include_directories(${MATS_INCLUDE_DIR})

#### google protobuf
if(WIN32)
set(PROTOBUF_INCLUDE_DIR E:/dev/3rdparty/google/protobuf/include)
set(PROTOBUF_LIBRARIES E:/dev/3rdparty/google/protobuf/lib/libprotobufd.lib)
endif(WIN32)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(CheckMats)
add_subdirectory(server)
#add_subdirectory(client)