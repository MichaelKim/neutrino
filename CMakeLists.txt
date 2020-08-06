cmake_minimum_required (VERSION 3.15)
project (neutrino)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

if(WIN32)
	set(CMAKE_WIN32_EXECUTABLE YES)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/duktape)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview)

add_executable(neutrino neutrino.cpp)
set_target_properties(neutrino PROPERTIES
		VS_PACKAGE_REFERENCES "Microsoft.Windows.CppWinRT_2.0.*")
target_link_libraries(neutrino PUBLIC ${NEUTRINO_LIBS} duktape json webview)
