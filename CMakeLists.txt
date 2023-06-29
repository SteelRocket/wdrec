cmake_minimum_required(VERSION 3.0.0)
project(wdrec VERSION 0.1.0)

# Source
add_subdirectory(${CMAKE_SOURCE_DIR}/src/http)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/keyreader)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/utils)


add_executable(wdrec 
    src/main.c
    src/data.c
    src/replword.c
    src/tagmap.c
    src/wcmds.c
    src/wparser.c
)

if(NOT WIN32)
    target_link_libraries(wdrec pthread)
endif()

target_link_libraries(wdrec wdrec_http wdrec_keyreader wdrec_utils)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
