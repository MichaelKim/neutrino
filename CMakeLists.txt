cmake_minimum_required (VERSION 3.1)
project (neutrino)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview ${CMAKE_CURRENT_BINARY_DIR}/webview)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann ${CMAKE_CURRENT_BINARY_DIR}/json)

add_executable(neutrino neutrino.cpp)
target_link_libraries(neutrino PUBLIC -lstdc++fs webview json)
