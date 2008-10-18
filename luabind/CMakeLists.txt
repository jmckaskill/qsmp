project(luabind)

find_package(Boost)

set(luabind_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "luabind include dir")

include_directories(${Boost_INCLUDE_DIR} ${lua_INCLUDE_DIR} ${luabind_INCLUDE_DIR})

set(sources src/class.cpp
            src/class_info.cpp
	    src/class_registry.cpp
	    src/class_rep.cpp
	    src/create_class.cpp
	    src/error.cpp
	    src/find_best_match.cpp
	    src/function.cpp
	    src/implicit_cast.cpp
	    src/link_compatibility.cpp
	    src/object_rep.cpp
	    src/open.cpp
	    src/overload_rep.cpp
	    src/pcall.cpp
	    src/ref.cpp
	    src/scope.cpp
	    src/stack_content_by_name.cpp
	    src/weak_ref.cpp
	    src/wrapper_base.cpp)


add_library(luabind STATIC ${sources})
