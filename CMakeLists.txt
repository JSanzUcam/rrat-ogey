cmake_minimum_required(VERSION 3.5.0)
project(Ogey VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(Ogey main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(${PROJECT_NAME} enet)