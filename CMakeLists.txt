cmake_minimum_required(VERSION 3.25.3)
project(GLFW-Vulkan-Example VERSION 0.0.1)

# Setting C++ Version to 2020
set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)

add_executable(${PROJECT_NAME}
  ${CMAKE_SOURCE_DIR}/include/stb_image/stb_image.h
  ${CMAKE_SOURCE_DIR}/include/stb_image/stb_image_imp.cpp
        ${CMAKE_SOURCE_DIR}/include/tiny_obj_loader/tiny_obj_loader.h
        ${CMAKE_SOURCE_DIR}/include/tiny_obj_loader/tiny_obj_loader_imp.cpp
  ${CMAKE_SOURCE_DIR}/include/application.hpp

  ${CMAKE_SOURCE_DIR}/src/main.cpp
  ${CMAKE_SOURCE_DIR}/src/application.cpp
)

add_custom_target(copy_resources ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${PROJECT_SOURCE_DIR}/resources
  ${PROJECT_BINARY_DIR}/resources
  COMMENT "Copying resources into binary directory"
)

add_dependencies(${PROJECT_NAME} copy_resources)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  glfw
  glm
  Vulkan::Vulkan
)

target_link_libraries(${PROJECT_NAME}
  glfw
  glm
  Vulkan::Vulkan
)