CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(PROJECT_TITLE "MazeRunner")
project(${PROJECT_TITLE} CXX)

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS net gl core base REQUIRED)
find_package(GLEW REQUIRED)
include(${wxWidgets_USE_FILE})
include_directories(${GLEW_INCLUDE_DIRS})

add_executable(${PROJECT_TITLE} main.cpp)

target_link_directories(${PROJECT_TITLE} PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(${PROJECT_TITLE} PRIVATE ${wxWidgets_LIBRARIES})
link_libraries(${PROJECT_TITLE} PRIVATE GLEW::GLEW)