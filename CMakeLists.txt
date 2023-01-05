cmake_minimum_required(VERSION 3.24)
project(bridge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

enable_testing()

ADD_SUBDIRECTORY(Tests)

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        Network
        REQUIRED)

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
        proxy/Clash.h
        proxy/Clash.cpp
        ui/NewProxyWidget.cpp
        ui/NewProxyWidget.h
        ui/NewProxyWidget.ui utils/Readable.cpp utils/Readable.h ui/SettingsWidget.cpp ui/SettingsWidget.h ui/SettingsWidget.ui proxy/IpInfo.cpp proxy/IpInfo.h)

target_link_libraries(bridge
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
        )

