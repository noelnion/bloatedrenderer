add_executable(intro main.cpp tgaimage.hpp tgaimage.cpp)

target_link_libraries(
  intro
  PRIVATE bloatedrenderer::bloatedrenderer_options
          bloatedrenderer::bloatedrenderer_warnings)

target_link_system_libraries(
  intro
  PRIVATE
          CLI11::CLI11
          fmt::fmt
          spdlog::spdlog
          lefticus::tools)

target_include_directories(intro PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
