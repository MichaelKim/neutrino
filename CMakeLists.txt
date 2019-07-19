cmake_minimum_required (VERSION 3.1)
if(APPLE)
  SET(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)
elseif(WIN32)
  SET(CMAKE_CXX_COMPILER clang-cl)
endif
project (neutrino)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann)

if(APPLE)
  include_directories(/usr/local/opt/llvm/include)
  link_directories(/usr/local/opt/llvm/lib)
  set(NEUTRINO_LIBS "-lc++fs")
else
  set(NEUTRINO_LIBS "-lstdc++fs")
endif

add_executable(neutrino neutrino.cpp)
target_link_libraries(neutrino PUBLIC ${NEUTRINO_LIBS} webview json)
