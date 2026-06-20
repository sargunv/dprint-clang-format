cmake_minimum_required(VERSION 4.0)

project(dprint_plugin_clang_format LANGUAGES C CXX)

set(LLVM_VERSION "22.1.7" CACHE STRING "LLVM source/build version")
set(LLVM_SRC "${PROJECT_SOURCE_DIR}/third_party/llvm-project-${LLVM_VERSION}.src" CACHE PATH
    "Patched LLVM source directory")
set(LLVM_WASM_BUILD "${PROJECT_SOURCE_DIR}/build/llvm-wasm" CACHE PATH
    "LLVM wasm build directory")
set(PLUGIN_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/build" CACHE PATH "Plugin output directory")

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXECUTABLE_SUFFIX ".wasm")

set(REQUIRED_LIBFORMAT_ARCHIVES
  "${LLVM_WASM_BUILD}/lib/libclangFormat.a"
  "${LLVM_WASM_BUILD}/lib/libclangToolingInclusions.a"
  "${LLVM_WASM_BUILD}/lib/libclangToolingCore.a"
  "${LLVM_WASM_BUILD}/lib/libclangRewrite.a"
  "${LLVM_WASM_BUILD}/lib/libclangLex.a"
  "${LLVM_WASM_BUILD}/lib/libclangBasic.a"
  "${LLVM_WASM_BUILD}/lib/libLLVMTargetParser.a"
  "${LLVM_WASM_BUILD}/lib/libLLVMSupport.a"
  "${LLVM_WASM_BUILD}/lib/libLLVMDemangle.a"
)

set(LIBCXX_SOURCES
  "${LLVM_SRC}/libcxx/src/chrono.cpp"
  "${LLVM_SRC}/libcxx/src/error_category.cpp"
  "${LLVM_SRC}/libcxx/src/functional.cpp"
  "${LLVM_SRC}/libcxx/src/hash.cpp"
  "${LLVM_SRC}/libcxx/src/memory.cpp"
  "${LLVM_SRC}/libcxx/src/new_helpers.cpp"
  "${LLVM_SRC}/libcxx/src/string.cpp"
  "${LLVM_SRC}/libcxx/src/system_error.cpp"
)

foreach(archive IN LISTS REQUIRED_LIBFORMAT_ARCHIVES)
  if(NOT EXISTS "${archive}")
    message(FATAL_ERROR "Required archive not found: ${archive}. Run: mise run build")
  endif()
endforeach()

add_executable(plugin
  src/dprint_plugin.cpp
  src/wasm_allocator.c
  src/wasm_runtime_core.cpp
  ${LIBCXX_SOURCES}
)

set_target_properties(plugin PROPERTIES
  OUTPUT_NAME "plugin"
  RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

target_compile_definitions(plugin PRIVATE
  CLANG_BUILD_STATIC=1
  _FILE_OFFSET_BITS=64
  _LARGEFILE_SOURCE
  __STDC_CONSTANT_MACROS
  __STDC_FORMAT_MACROS
  __STDC_LIMIT_MACROS
  _LIBCPP_DISABLE_EXTERN_TEMPLATE
)

target_compile_options(plugin PRIVATE
  -Os
  -ffreestanding
  -ffunction-sections
  -fdata-sections
  $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
  $<$<COMPILE_LANGUAGE:CXX>:-fvisibility=hidden>
  $<$<COMPILE_LANGUAGE:CXX>:-nostdinc++>
)

target_compile_options(plugin PRIVATE
  $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANG_AND_ID:CXX,Clang>>:-Wno-unused-command-line-argument>
)

target_include_directories(plugin SYSTEM PRIVATE
  "${PROJECT_SOURCE_DIR}/support/libcxx-wasm/include"
  "${LLVM_SRC}/libcxx/include"
  "${LLVM_SRC}/libcxx/src/include"
  "${PROJECT_SOURCE_DIR}/support/wasm-sysroot/include"
)

target_include_directories(plugin PRIVATE
  "${LLVM_SRC}/clang/include"
  "${LLVM_WASM_BUILD}/tools/clang/include"
  "${LLVM_WASM_BUILD}/include"
  "${LLVM_SRC}/llvm/include"
)

set_source_files_properties(${LIBCXX_SOURCES} PROPERTIES
  COMPILE_DEFINITIONS "_LIBCPP_BUILDING_LIBRARY"
)

set(DPRINT_PLUGIN_EXPORTS
  dprint_plugin_version_4
  get_shared_bytes_ptr
  clear_shared_bytes
  get_plugin_info
  get_license_text
  register_config
  release_config
  get_config_diagnostics
  get_resolved_config
  get_config_file_matching
  set_file_path
  set_override_config
  format
  format_range
  get_formatted_text
  get_error_text
  check_config_updates
)

set(PLUGIN_LINK_OPTIONS
  -nostdlib
  LINKER:--no-entry
  LINKER:--initial-memory=67108864
  LINKER:--max-memory=67108864
  LINKER:--export=malloc
  LINKER:--export=free
  LINKER:--export-memory
  LINKER:--gc-sections
)

foreach(export_name IN LISTS DPRINT_PLUGIN_EXPORTS)
  list(APPEND PLUGIN_LINK_OPTIONS "LINKER:--export=${export_name}")
endforeach()

target_link_options(plugin PRIVATE ${PLUGIN_LINK_OPTIONS})

# wasm-ld has no --start-group; two passes resolve circular refs between these archives.
target_link_libraries(plugin PRIVATE
  ${REQUIRED_LIBFORMAT_ARCHIVES}
  ${REQUIRED_LIBFORMAT_ARCHIVES}
)

find_program(WASM_VALIDATE_EXECUTABLE wasm-validate REQUIRED)
add_custom_command(TARGET plugin POST_BUILD
  COMMAND "${WASM_VALIDATE_EXECUTABLE}" "$<TARGET_FILE:plugin>"
  COMMAND "${CMAKE_COMMAND}" -E echo "built $<TARGET_FILE:plugin>"
  VERBATIM
)
