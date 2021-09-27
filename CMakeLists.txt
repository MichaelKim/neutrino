cmake_minimum_required (VERSION 3.15)
project(neutrino
  VERSION 1.0.0
	LANGUAGES CXX
  DESCRIPTION
    "Build cross-platform desktop applications using your OS' native web engine."
  HOMEPAGE_URL "https://github.com/MichaelKim/neutrino")

# Build Settings
include(cmake/Settings.cmake)

# Sanitizers
add_library(project_options INTERFACE)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# Warnings
add_library(project_warnings INTERFACE)
include(cmake/Warnings.cmake)
set_project_warnings(project_warnings)

# Static Analysis
include(cmake/StaticAnalyzers.cmake)

# Libraries
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/duktape)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview)

add_executable(${PROJECT_NAME} neutrino.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES
  WIN32_EXECUTABLE ${WIN32}
  # Remove subdirectory on Windows (e.g. Debug/, Release/)
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/$<0:>)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE project_options project_warnings duktape json webview)

if (WEBVIEW_STATIC_WEBVIEW2)
  # Copy WebView2Loader.dll if webview is statically linked
  add_custom_command(
    TARGET ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/WebView2Loader.dll
            ${CMAKE_SOURCE_DIR}/lib)
else()
  # Delete WebView2Loader.dll if webview is dynamically linked
  add_custom_command(
    TARGET ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_SOURCE_DIR}/lib/WebView2Loader.dll)
endif()