# Copyright (C) 2018-2020 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

set (TARGET_NAME "clDNNPlugin")

if (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1 -msse4.2")
    if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=terminate")
    endif()
endif()

if(ENABLE_LTO)
    ie_enable_lto()
endif()

file(GLOB MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB LIBRARY_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

addVersionDefines(cldnn_engine.cpp CI_BUILD_NUMBER CLDNN_VERSION)

ie_add_plugin(NAME ${TARGET_NAME}
              DEVICE_NAME "GPU"
              SOURCES ${MAIN_SRC} ${LIBRARY_HEADERS}
              VERSION_DEFINES_FOR cldnn_engine.cpp)

target_link_libraries(${TARGET_NAME} PRIVATE ${INTEL_ITT_LIBS} inference_engine clDNN_lib pugixml)

set (CLDNN_TOP_FOLDER ${IE_MAIN_SOURCE_DIR}/thirdparty/clDNN)
target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CLDNN__IOCL_ICD_INCDIRS}
        ${CLDNN_TOP_FOLDER}
        ${IE_MAIN_SOURCE_DIR}/thirdparty/pugixml/src)

# copy default global xml file describing the custom kernels and the *.cl files

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/cldnn_global_custom_kernels
                                                 $<TARGET_FILE_DIR:${TARGET_NAME}>/cldnn_global_custom_kernels)

# install

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cldnn_global_custom_kernels
        DESTINATION ${IE_CPACK_LIBRARY_PATH}
        COMPONENT gpu)

install(FILES "${clDNN_SOURCE_DIR}/kernel_selector/core/cache/cache.json"
        DESTINATION ${IE_CPACK_LIBRARY_PATH}
        COMPONENT gpu)
