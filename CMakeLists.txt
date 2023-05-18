cmake_minimum_required(VERSION 3.12)
project(RealmeGPSL5Enabler
    VERSION "0.0.1"
    DESCRIPTION "Enable L5/E5a/B2a GPS bands on realme devices."
    HOMEPAGE_URL "https://github.com/rapperskull/realme_gps_l5_enabler/"
    LANGUAGES C
)

add_subdirectory(libnvbk EXCLUDE_FROM_ALL)

set(sources_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(gnss_patcher ${sources_dir}/gnss_patcher.c)
if(WIN32)
    target_sources(gnss_patcher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/getopt/getopt.h)
    target_include_directories(gnss_patcher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/getopt)
endif()
set_property(TARGET gnss_patcher PROPERTY C_STANDARD 99)
set_property(TARGET gnss_patcher PROPERTY C_EXTENSIONS ON)
if(MSVC)
    target_compile_options(gnss_patcher PUBLIC /W4)
else()
    target_compile_options(gnss_patcher PUBLIC -Wall PUBLIC -Wextra PUBLIC -Wpedantic)
endif()
target_link_libraries(gnss_patcher nvbk)

if(ANDROID)
    set(MAGISK_LONG_NAME "realme GPS L5 Enabler")
    set(MAGISK_VERSION_CODE 1)
    set(MAGISK_UPDATE_JSON "https://raw.githubusercontent.com/rapperskull/realme_gps_l5_enabler/master/update.json")

    set(MAGISK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/magisk/)
    set(MAGISK_FILES
        META-INF/com/google/android/update-binary
        META-INF/com/google/android/updater-script
        post-fs-data.sh
    )
    list(TRANSFORM MAGISK_FILES PREPEND ${MAGISK_DIR} OUTPUT_VARIABLE MAGISK_SRC)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/module.prop.in ${CMAKE_CURRENT_BINARY_DIR}/module.prop @ONLY)
    add_custom_target(
        create_release ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${MAGISK_DIR} ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E tar cf ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-v${PROJECT_VERSION}.zip --format=zip ${MAGISK_FILES} module.prop gnss_patcher LICENSE
        DEPENDS gnss_patcher ${CMAKE_CURRENT_BINARY_DIR}/module.prop
        SOURCES ${MAGISK_SRC}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
