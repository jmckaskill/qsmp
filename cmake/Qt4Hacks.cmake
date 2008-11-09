
   MACRO(QSMP_QT4_AUTOMOC)
      QT4_GET_MOC_INC_DIRS(_moc_INCS)

      SET(_matching_FILES )
      FOREACH (_current_FILE ${ARGN})

         GET_FILENAME_COMPONENT(_abs_FILE ${_current_FILE} ABSOLUTE)
         # if "SKIP_AUTOMOC" is set to true, we will not handle this file here.
         # This is required to make uic work correctly:
         # we need to add generated .cpp files to the sources (to compile them),
         # but we cannot let automoc handle them, as the .cpp files don't exist yet when
         # cmake is run for the very first time on them -> however the .cpp files might
         # exist at a later run. at that time we need to skip them, so that we don't add two
         # different rules for the same moc file
         GET_SOURCE_FILE_PROPERTY(_skip ${_abs_FILE} SKIP_AUTOMOC)

         IF ( NOT _skip AND EXISTS ${_abs_FILE} )

            FILE(READ ${_abs_FILE} _contents)

            GET_FILENAME_COMPONENT(_abs_PATH ${_abs_FILE} PATH)

            STRING(REGEX MATCHALL "#include +[^ ]+\\.moc[\">]" _match "${_contents}")
            IF(_match)
               FOREACH (_current_MOC_INC ${_match})
                  STRING(REGEX MATCH "[^ <\"]+\\.moc" _current_MOC "${_current_MOC_INC}")
                  GET_FILENAME_COMPONENT(_basename ${_current_MOC} NAME_WE)
                  GET_FILENAME_COMPONENT(_path     ${_current_MOC} PATH)
		  GET_DIRECTORY_PROPERTY(_includes INCLUDE_DIRECTORIES)
		  SET(_header "")
		  FOREACH(_include_path ${_includes})
                     SET(_header_path ${_include_path}/${_path}/${_basename}.h)
                     FILE(GLOB _exists_test ${_header_path})
		     IF(_exists_test)
                        SET(_header ${_header_path})
			BREAK()
                     ENDIF(_exists_test)
                  ENDFOREACH(_include_path)
		  IF(_header)
                     SET(_moc    ${CMAKE_CURRENT_BINARY_DIR}/moc/${_current_MOC})
                     QT4_CREATE_MOC_COMMAND(${_header} ${_moc} "${_moc_INCS}" "")
                     MACRO_ADD_FILE_DEPENDENCIES(${_abs_FILE} ${_moc})
		  ENDIF(_header)
               ENDFOREACH (_current_MOC_INC)
            ENDIF(_match)
         ENDIF ( NOT _skip AND EXISTS ${_abs_FILE} )
      ENDFOREACH (_current_FILE)
   ENDMACRO(QSMP_QT4_AUTOMOC)
