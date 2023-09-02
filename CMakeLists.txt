cmake_minimum_required(VERSION 3.25.3)
project(GLFW-Vulkan-Example VERSION 0.0.1)

# Setting C++ Version to 2020
set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

if (WIN32)
  set(VULKAN_INCLUDE_DIR "$ENV{VULKAN_SDK}/Include")
  set(VULKAN_LIBRARY "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")
endif()

if(APPLE)
  set(VULKAN_INCLUDE_DIR "$ENV{VULKAN_SDK}/include")
  set(VULKAN_LIBRARY "$ENV{VULKAN_SDK}/lib/libvulkan.1.3.250.dylib")
  set(VULKAN_ICD_FILENAMES "$ENV{VULKAN_SDK}/share/vulkan/icd.d/MoltenVK_icd.json")
  set(VULKAN_LAYER_PATH "$ENV{VULKAN_SDK}/share/vulkan/explicit_layer.d")
endif()

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)

add_executable(${PROJECT_NAME}
  ${CMAKE_SOURCE_DIR}/include/application.hpp

  ${CMAKE_SOURCE_DIR}/src/main.cpp
  ${CMAKE_SOURCE_DIR}/src/application.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  glfw
  glm
  ${VULKAN_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  glfw
  glm
  ${VULKAN_LIBRARY}
)

# Add the VK_ICD_FILENAMES and VK_LAYER_PATH environment variables as compile definitions
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  VK_ICD_FILENAMES="${VULKAN_ICD_FILENAMES}"
  VK_LAYER_PATH="${VULKAN_LAYER_PATH}"
)