include_external_msproject(zlib   ${CMAKE_CURRENT_SOURCE_DIR}/zlib/prj/zlib.vcproj)
include_external_msproject(id3lib ${CMAKE_CURRENT_SOURCE_DIR}/libprj/id3lib.vcproj)
set(id3lib_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)