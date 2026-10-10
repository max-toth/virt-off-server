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
)

set(CLIENTS_SOURCES
    src/clients/ClientRepository.cpp
)

set(NET_SOURCES
    src/net/UdpTransport.cpp
    src/net/PacketRouter.cpp
)

set(HANDLERS_SOURCES
    src/handlers/ConnectHandler.cpp
    src/handlers/RelayHandler.cpp
)

set(APP_SOURCES
    src/app/Server.cpp
    src/app/ServerBuilder.cpp
    src/app/ServerFactory.cpp
    src/app/CliParser.cpp
)

set(V2_SOURCES
    src/v2/core/TimestampIdGenerator.cpp
    src/v2/clients/UserRepository.cpp
    src/v2/handlers/ConnectHandler.cpp
    src/v2/handlers/RelayHandler.cpp
    src/v2/app/Server.cpp
    src/v2/app/ServerBuilder.cpp
)

# ---- Library ----
add_library(virt-off-server-core STATIC
    ${CORE_SOURCES}
    ${CLIENTS_SOURCES}
    ${NET_SOURCES}
    ${HANDLERS_SOURCES}
    ${APP_SOURCES}
    ${V2_SOURCES}
)

target_include_directories(virt-off-server-core PUBLIC src)
target_include_directories(virt-off-server-core PUBLIC ${asio_SOURCE_DIR}/asio/include)

# ---- Executable ----
add_executable(virt-off-server src/main.cpp)
target_link_libraries(virt-off-server PRIVATE virt-off-server-core Threads::Threads)

# ---- Tests ----
enable_testing()
add_test(
    NAME smoke_server
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke_server_test.sh $<TARGET_FILE:virt-off-server>
)

# ---- Integration test client (out of the server project code) ----
add_executable(test_client tests/integration/test_client.cpp)
target_include_directories(test_client PRIVATE ${asio_SOURCE_DIR}/asio/include)
target_link_libraries(test_client PRIVATE Threads::Threads)

# Multiplayer integration test: real v2 server + listener/sender clients.
# Registered in CTest but NOT run automatically on every build (slower,
# timing/network sensitive) — run it explicitly with: ctest -R integration_server
add_test(
    NAME integration_server
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/integration_test.sh
            $<TARGET_FILE:virt-off-server> $<TARGET_FILE:test_client>
)

# Re-run the smoke tests after every build. Skipped for cross-arch builds
# (e.g. x86_64 on Apple Silicon) where the produced binary cannot run locally.
if(NOT CMAKE_OSX_ARCHITECTURES)
    add_custom_target(run_smoke_tests ALL
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R smoke_server
        DEPENDS virt-off-server test_client
        COMMENT "Running smoke tests"
    )
endif()
