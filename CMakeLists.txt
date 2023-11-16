cmake_minimum_required(VERSION 3.24)
project(bridge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

enable_testing()

ADD_SUBDIRECTORY(Tests)

find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
set(PROTO_FILES
        proxy/v2ray_stats_grpc.proto
        )
add_library(v2ray_stats ${PROTO_FILES})
target_link_libraries(v2ray_stats
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(v2ray_stats PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate(TARGET v2ray_stats LANGUAGE cpp)
protobuf_generate(TARGET v2ray_stats LANGUAGE grpc
        GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
        PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        Network
        REQUIRED)

add_compile_definitions(CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

add_executable(
        bridge
        bridge.qrc
        main.cpp
        ui/MainWindow.cpp
        ui/MainWindow.h
        ui/MainWindow.ui
        network/Http.cpp
        network/Http.h
        proxy/Proxy.cpp
        proxy/Proxy.h
        proxy/Provider.cpp
        proxy/Provider.h
        utils/Config.cpp
        utils/Config.h
        ui/ProviderEditorWidget.cpp
        ui/ProviderEditorWidget.h
        ui/ProviderEditorWidget.ui
        proxy/ProxyCmd.h
        proxy/ProxyCmd.cpp
        ui/NewProxyWidget.cpp
        ui/NewProxyWidget.h
        ui/NewProxyWidget.ui
        utils/Readable.cpp
        utils/Readable.h
        ui/SettingsWidget.cpp
        ui/SettingsWidget.h
        ui/SettingsWidget.ui
        proxy/IpInfo.cpp
        proxy/IpInfo.h
        proxy/v2ray_stats_grpc.pb.h
        proxy/v2ray_stats_grpc.pb.cc)

target_link_libraries(bridge
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
        v2ray_stats
        )

