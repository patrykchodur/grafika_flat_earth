cmake_minimum_required(VERSION 3.7)
project(zajecia3)

find_package(SFML COMPONENTS REQUIRED graphics window system)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g -Wunused-const-variable")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(source_files pch.cpp test.cpp materials.cpp objloader.cpp)
set(assets shader_3.frag shader_3.vert flat_earth.obj skybox.jpg flat_earth_texture.png water.jpg)

add_executable(${PROJECT_NAME} ${source_files})

# target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${SFML_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${GLUT_INCLUDE_DIRS})


target_link_libraries(${PROJECT_NAME} sfml-graphics)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${GLUT_LIBRARY})

foreach (file ${assets})
	configure_file(${file} ${file} COPYONLY)
endforeach()
