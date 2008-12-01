project(qsmp_lib)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS filesystem date_time)

set(source Log.cpp)
set(headers Log.h)

include_directories(${Boost_INCLUDE_DIR})

add_library(qsmp_lib STATIC ${source} ${headers})
