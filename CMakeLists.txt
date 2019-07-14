cmake_minimum_required (VERSION 3.1)
project (neutrino)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann)

add_executable(neutrino neutrino.cpp)
target_link_libraries(neutrino PUBLIC -lc++fs webview json)
