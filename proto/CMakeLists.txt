cmake_minimum_required(VERSION 3.16)

project(dcm_proto VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# The .proto files are the only sources listed: the .pb.cc / .grpc.pb.cc are
# added to the target by protobuf_generate below.
add_library(dcm_proto STATIC
    config_service.proto
    storage_service.proto)

# Messages: config_service.pb.h / .pb.cc
protobuf_generate(TARGET dcm_proto LANGUAGE cpp)

# Services: config_service.grpc.pb.h / .grpc.pb.cc — the Stub used by clients
# and the Service base class implemented by servers. Without the plugin the
# service blocks in the .proto are silently ignored.
protobuf_generate(TARGET dcm_proto LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>")

# Generated headers live in the build tree, not next to the .proto files.
target_include_directories(dcm_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(dcm_proto PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++)
