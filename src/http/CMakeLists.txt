add_library(wdrec_http
    http.c
    http_client.c
    http_api.c
)


if(WIN32)
    target_link_libraries(wdrec_http wsock32 ws2_32)
endif()
