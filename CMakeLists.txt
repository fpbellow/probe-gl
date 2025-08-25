# CMakeList.txt : CMake project for global-illum, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("global-illum")


find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(OpenEXR CONFIG REQUIRED)
find_package(simdjson REQUIRED)

# Add source to this project's executable.
add_executable (${PROJECT_NAME} "global-illum.cpp" "src/Application.cpp" "src/D3D11Application.cpp" "src/GeometryDefs.cpp" "src/ResourceLoader.cpp" "src/ShaderCollection.cpp" "src/InputHandler.cpp" "src/ImGuiMenu.cpp"     "src/Camera.cpp" "src/D3DBufferDefs.cpp")

target_link_libraries(${PROJECT_NAME} PRIVATE glfw imgui::imgui fastgltf::fastgltf OpenEXR::OpenEXR simdjson::simdjson)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
