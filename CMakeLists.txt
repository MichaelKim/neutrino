cmake_minimum_required (VERSION 3.15)
project (neutrino LANGUAGES CXX)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/duktape)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview)

set(CMAKE_WIN32_EXECUTABLE ${WIN32})

add_executable(neutrino neutrino.cpp)
target_compile_features(neutrino PRIVATE cxx_std_17)
target_link_libraries(neutrino PRIVATE ${NEUTRINO_LIBS} duktape json webview)
