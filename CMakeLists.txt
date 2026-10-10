cmake_minimum_required(VERSION 3.20)
project(virt-off-server VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# ---- Fetch asio (standalone, header-only) ----
include(FetchContent)
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG        asio-1-30-2
)
FetchContent_MakeAvailable(asio)

# ---- Sources ----
set(CORE_SOURCES
    src/core/ConsoleLogger.cpp
    src/core/MonotonicIdGenerator.cpp
    src/core/TimestampIdGeneratorV2.cpp
)

set(CLIENTS_SOURCES
    src/clients/ClientRepository.cpp
    src/clients/UserRepositoryV2.cpp
)

set(NET_SOURCES
    src/net/UdpTransport.cpp
    src/net/PacketRouter.cpp
)

set(HANDLERS_SOURCES
    src/handlers/ConnectHandler.cpp
    src/handlers/ConnectHandlerV2.cpp
    src/handlers/RelayHandler.cpp
)

set(APP_SOURCES
    src/app/Server.cpp
)

# ---- Library ----
add_library(virt-off-server-core STATIC
    ${CORE_SOURCES}
    ${CLIENTS_SOURCES}
    ${NET_SOURCES}
    ${HANDLERS_SOURCES}
    ${APP_SOURCES}
)

target_include_directories(virt-off-server-core PUBLIC src)
target_include_directories(virt-off-server-core PUBLIC ${asio_SOURCE_DIR}/asio/include)

# ---- Executable ----
add_executable(virt-off-server src/main.cpp)
target_link_libraries(virt-off-server PRIVATE virt-off-server-core Threads::Threads)
