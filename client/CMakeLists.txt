find_package(Boost REQUIRED COMPONENTS date_time system)

include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(${MATS_INCLUDE_DIR})

# Set executable source set
set(ALL_SRC main.cpp ${CMAKE_SOURCE_DIR}/mats/mats.pb.cc)

# Generate executable file
add_executable(client ${ALL_SRC})

target_link_libraries(client ${PROTOBUF_LIBRARIES})

install(TARGETS client DESTINATION ${CMAKE_BINARY_DIR}/bin)