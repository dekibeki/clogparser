cmake_minimum_required (VERSION 3.8)

PROJECT(clogparser CXX)

add_library(clogparser STATIC
  "src/parser.cpp" 
  "src/types.cpp"
  "src/clogparser.cpp" 
  "src/item.cpp")

target_include_directories(clogparser PUBLIC
  "include_public")

target_include_directories(clogparser PUBLIC
  "include_private")

target_link_libraries(clogparser PUBLIC common)

target_compile_features(clogparser PUBLIC
	cxx_std_20)

IF(${VCPKG_TARGET_TRIPLET} MATCHES ".*-static")
  set_property(TARGET clogparser PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
ENDIF()