cmake_minimum_required(VERSION 3.2)

project(game)
set(OUT ${PROJECT_NAME})

add_executable(${OUT} src/main.cpp)

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
target_link_libraries(${OUT} PRIVATE sfml-system sfml-network sfml-graphics sfml-window)
# If you want SFML to provide an implementation of main():
# target_link_libraries(${OUT} PRIVATE sfml-main)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${OUT} PRIVATE imgui::imgui)

find_package(ImGui-SFML CONFIG REQUIRED)
target_link_libraries(${OUT} PRIVATE ImGui-SFML::ImGui-SFML)
