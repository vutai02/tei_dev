cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_CURRENT_LIST_DIR}/files.cmake)

idf_component_register(SRCS ${SOURCES}
  INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}
    $ENV{IDF_PATH}/components
  REQUIRES
    smooth_component
    esp_http_client
    app_update
    spi_flash
  EMBED_TXTFILES ${CMAKE_CURRENT_LIST_DIR}/server_certs/ca_cert.pem
  )