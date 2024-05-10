cmake_minimum_required(VERSION 3.15)
project(FlutterEmbedderFB)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
set(FLUTTER_ENGINE_VARIANT "host_debug_unopt" CACHE STRING "")

add_executable(flutter_fbdev src/flutter_fbdev.cc)

include_directories(${CMAKE_SOURCE_DIR}/third_party/flutter_embedder_header/include)

find_library(FLUTTER_LIB flutter_engine PATHS
	${CMAKE_SOURCE_DIR}/../../../out/${FLUTTER_ENGINE_VARIANT}
        ${STAGING_DIR_TARGET}${datadir}/flutter/${FLUTTER_SDK_VERSION}/${FLUTTER_RUNTIME_MODE}/lib/
	${CMAKE_SOURCE_DIR}
)

target_link_libraries(flutter_fbdev ${FLUTTER_LIB} ${CMAKE_DL_LIBS})

add_custom_command(
        TARGET flutter_fbdev POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${FLUTTER_LIB}
                ${CMAKE_CURRENT_BINARY_DIR})
