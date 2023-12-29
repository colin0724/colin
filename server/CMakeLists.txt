find_package(Boost REQUIRED COMPONENTS date_time system)

include_directories(../CheckMats)
include_directories(${PROTOBUF_INCLUDE_DIR})

# Set executable source set
set(ALL_SRC main.cpp server.cpp mats.cpp camera.cpp ${CMAKE_SOURCE_DIR}/mats/mats.pb.cc)

# Generate executable file
add_executable(server ${ALL_SRC})

target_link_libraries(server CheckMats)
target_link_libraries(server ${PROTOBUF_LIBRARIES})

install(TARGETS server DESTINATION ${CMAKE_BINARY_DIR}/bin)