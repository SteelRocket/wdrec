cmake_minimum_required(VERSION 3.0.0)
project(wdrec VERSION 0.1.1)

add_compile_definitions(WDREC_VERSION="${PROJECT_VERSION}")

add_subdirectory(${CMAKE_SOURCE_DIR}/src/http)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/keyreader)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/utils)

add_executable(wdrec 
    src/main.c
    src/data.c
    src/replword.c

    src/tagmap.c
    src/tagvalid.c
    
    src/wcmds.c
    src/wparser.c
)

if(NOT WIN32)
    target_link_libraries(wdrec pthread)
endif()

target_link_libraries(wdrec wdrec_http wdrec_keyreader wdrec_utils)

install(TARGETS wdrec DESTINATION .)

set(CPACK_GENERATOR ZIP)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

set(CPACK_PACKAGE_VENDOR "SteelRocket")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

include(CPack)