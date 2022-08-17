# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# Obter a tag de versão do GIT
find_package (Git)
if (GIT_FOUND)
      message("git found: ${GIT_EXECUTABLE} in version ${GIT_VERSION_STRING}")
      
      execute_process(
          COMMAND           "${GIT_EXECUTABLE}" describe --tags
          WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
          RESULT_VARIABLE   git_result
          OUTPUT_VARIABLE   git_tag
          ERROR_VARIABLE    git_error
          OUTPUT_STRIP_TRAILING_WHITESPACE
          ERROR_STRIP_TRAILING_WHITESPACE
      )
endif (GIT_FOUND)


#Versão do firmware
set(PROJECT_VER ${git_tag})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(cp32eth_nucd)

add_custom_command(
    PRE_BUILD 
    COMMAND bash -c "source ./script/compress_server.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    TARGET app
)

#Salva os arquivos na flash interna do ESP32
spiffs_create_partition_image(storage0 "flash_partitions/server" FLASH_IN_PROJECT)
spiffs_create_partition_image(storage1 "flash_partitions/data" FLASH_IN_PROJECT)
