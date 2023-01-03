set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Test Widgets Network)

ADD_EXECUTABLE(ProxyTest proxy.cpp ../../proxy/Proxy.cpp )
target_link_libraries(ProxyTest PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Test
        Qt::Network
        Qt::Widgets
        )

ADD_TEST(NAME ProxyTest COMMAND ProxyTest)
