project(qsmp_indexer)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS filesystem system program_options)

include_directories(${Boost_INCLUDE_DIR}
                    ${id3lib_INCLUDE_DIR})

add_definitions(-DID3LIB_LINKOPTION=1)

set(sources qsmp_indexer.cpp)


add_executable(qsmp_indexer ${sources})

target_link_libraries(qsmp_indexer
                      ${Boost_LIBRARIES}
                      id3lib
                      zlib)