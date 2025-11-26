include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


include(CheckCXXSourceCompiles)


macro(bloatedrenderer_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)

    message(STATUS "Sanity checking UndefinedBehaviorSanitizer, it should be supported on this platform")
    set(TEST_PROGRAM "int main() { return 0; }")

    # Check if UndefinedBehaviorSanitizer works at link time
    set(CMAKE_REQUIRED_FLAGS "-fsanitize=undefined")
    set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=undefined")
    check_cxx_source_compiles("${TEST_PROGRAM}" HAS_UBSAN_LINK_SUPPORT)

    if(HAS_UBSAN_LINK_SUPPORT)
      message(STATUS "UndefinedBehaviorSanitizer is supported at both compile and link time.")
      set(SUPPORTS_UBSAN ON)
    else()
      message(WARNING "UndefinedBehaviorSanitizer is NOT supported at link time.")
      set(SUPPORTS_UBSAN OFF)
    endif()
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    if (NOT WIN32)
      message(STATUS "Sanity checking AddressSanitizer, it should be supported on this platform")
      set(TEST_PROGRAM "int main() { return 0; }")

      # Check if AddressSanitizer works at link time
      set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
      set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address")
      check_cxx_source_compiles("${TEST_PROGRAM}" HAS_ASAN_LINK_SUPPORT)

      if(HAS_ASAN_LINK_SUPPORT)
        message(STATUS "AddressSanitizer is supported at both compile and link time.")
        set(SUPPORTS_ASAN ON)
      else()
        message(WARNING "AddressSanitizer is NOT supported at link time.")
        set(SUPPORTS_ASAN OFF)
      endif()
    else()
      set(SUPPORTS_ASAN ON)
    endif()
  endif()
endmacro()

macro(bloatedrenderer_setup_options)
  option(bloatedrenderer_ENABLE_HARDENING "Enable hardening" ON)
  option(bloatedrenderer_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    bloatedrenderer_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    bloatedrenderer_ENABLE_HARDENING
    OFF)

  bloatedrenderer_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR bloatedrenderer_PACKAGING_MAINTAINER_MODE)
    option(bloatedrenderer_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(bloatedrenderer_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(bloatedrenderer_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(bloatedrenderer_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(bloatedrenderer_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(bloatedrenderer_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(bloatedrenderer_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(bloatedrenderer_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(bloatedrenderer_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(bloatedrenderer_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(bloatedrenderer_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(bloatedrenderer_ENABLE_PCH "Enable precompiled headers" OFF)
    option(bloatedrenderer_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(bloatedrenderer_ENABLE_IPO "Enable IPO/LTO" ON)
    option(bloatedrenderer_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(bloatedrenderer_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(bloatedrenderer_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(bloatedrenderer_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(bloatedrenderer_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(bloatedrenderer_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(bloatedrenderer_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(bloatedrenderer_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(bloatedrenderer_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(bloatedrenderer_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(bloatedrenderer_ENABLE_PCH "Enable precompiled headers" OFF)
    option(bloatedrenderer_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      bloatedrenderer_ENABLE_IPO
      bloatedrenderer_WARNINGS_AS_ERRORS
      bloatedrenderer_ENABLE_USER_LINKER
      bloatedrenderer_ENABLE_SANITIZER_ADDRESS
      bloatedrenderer_ENABLE_SANITIZER_LEAK
      bloatedrenderer_ENABLE_SANITIZER_UNDEFINED
      bloatedrenderer_ENABLE_SANITIZER_THREAD
      bloatedrenderer_ENABLE_SANITIZER_MEMORY
      bloatedrenderer_ENABLE_UNITY_BUILD
      bloatedrenderer_ENABLE_CLANG_TIDY
      bloatedrenderer_ENABLE_CPPCHECK
      bloatedrenderer_ENABLE_COVERAGE
      bloatedrenderer_ENABLE_PCH
      bloatedrenderer_ENABLE_CACHE)
  endif()

  bloatedrenderer_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (bloatedrenderer_ENABLE_SANITIZER_ADDRESS OR bloatedrenderer_ENABLE_SANITIZER_THREAD OR bloatedrenderer_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(bloatedrenderer_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(bloatedrenderer_global_options)
  if(bloatedrenderer_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    bloatedrenderer_enable_ipo()
  endif()

  bloatedrenderer_supports_sanitizers()

  if(bloatedrenderer_ENABLE_HARDENING AND bloatedrenderer_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR bloatedrenderer_ENABLE_SANITIZER_UNDEFINED
       OR bloatedrenderer_ENABLE_SANITIZER_ADDRESS
       OR bloatedrenderer_ENABLE_SANITIZER_THREAD
       OR bloatedrenderer_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${bloatedrenderer_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${bloatedrenderer_ENABLE_SANITIZER_UNDEFINED}")
    bloatedrenderer_enable_hardening(bloatedrenderer_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(bloatedrenderer_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(bloatedrenderer_warnings INTERFACE)
  add_library(bloatedrenderer_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  bloatedrenderer_set_project_warnings(
    bloatedrenderer_warnings
    ${bloatedrenderer_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(bloatedrenderer_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    bloatedrenderer_configure_linker(bloatedrenderer_options)
  endif()

  include(cmake/Sanitizers.cmake)
  bloatedrenderer_enable_sanitizers(
    bloatedrenderer_options
    ${bloatedrenderer_ENABLE_SANITIZER_ADDRESS}
    ${bloatedrenderer_ENABLE_SANITIZER_LEAK}
    ${bloatedrenderer_ENABLE_SANITIZER_UNDEFINED}
    ${bloatedrenderer_ENABLE_SANITIZER_THREAD}
    ${bloatedrenderer_ENABLE_SANITIZER_MEMORY})

  set_target_properties(bloatedrenderer_options PROPERTIES UNITY_BUILD ${bloatedrenderer_ENABLE_UNITY_BUILD})

  if(bloatedrenderer_ENABLE_PCH)
    target_precompile_headers(
      bloatedrenderer_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(bloatedrenderer_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    bloatedrenderer_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(bloatedrenderer_ENABLE_CLANG_TIDY)
    bloatedrenderer_enable_clang_tidy(bloatedrenderer_options ${bloatedrenderer_WARNINGS_AS_ERRORS})
  endif()

  if(bloatedrenderer_ENABLE_CPPCHECK)
    bloatedrenderer_enable_cppcheck(${bloatedrenderer_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(bloatedrenderer_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    bloatedrenderer_enable_coverage(bloatedrenderer_options)
  endif()

  if(bloatedrenderer_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(bloatedrenderer_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(bloatedrenderer_ENABLE_HARDENING AND NOT bloatedrenderer_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR bloatedrenderer_ENABLE_SANITIZER_UNDEFINED
       OR bloatedrenderer_ENABLE_SANITIZER_ADDRESS
       OR bloatedrenderer_ENABLE_SANITIZER_THREAD
       OR bloatedrenderer_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    bloatedrenderer_enable_hardening(bloatedrenderer_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
