project(qsmp_indexer)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS filesystem system program_options)


if(USE_BUILTIN_ID3LIB)
  include_directories(${id3lib_INCLUDE_DIR})
  add_definitions(-DID3LIB_LINKOPTION=1)
else(USE_BUILTIN_ID3LIB)
  find_package(ID3 REQUIRED)
endif(USE_BUILTIN_ID3LIB)

include_directories(${Boost_INCLUDE_DIR}
                    ${ID3_INCLUDE_DIR})

set(sources qsmp_indexer.cpp)


add_executable(qsmp_indexer ${sources})

target_link_libraries(qsmp_indexer
                      ${Boost_LIBRARIES}
                      ${ID3_LIBRARY}
                     )

if(USE_BUILTIN_ID3LIB)
  target_link_libraries(id3lib zlib)
endif(USE_BUILTIN_ID3LIB)
