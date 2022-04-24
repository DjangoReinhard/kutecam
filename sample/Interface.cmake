#
# common settings for FalconView
#
include(GNUInstallDirs)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_BUILD_TYPE  Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_definitions(USE_PLUGINS)
add_compile_definitions(ULAPI)
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)
add_compile_definitions(QT_USE_QSTRINGBUILDER)

#--------------------------------------------------------------
# replacement for usage of interface library definition
#--------------------------------------------------------------
function(use_interface_libraries)
  list(POP_FRONT ARGV tgt)
  message("processing target: ${tgt}")
  if(NOT TARGET ${tgt})
     message(FATAL_ERROR "There is no target named '${tgt}'")
     return()
  endif()
  foreach(ifl ${ARGV})
          message(" - add interface lib: ${ifl}")
          if(NOT TARGET ${ifl})
             message(FATAL_ERROR "interface library '${ifl}' not found!")
             return()
          endif()
          get_target_property(_IFL_TYPE ${ifl} TYPE)
          if(NOT ${_IFL_TYPE} STREQUAL "INTERFACE_LIBRARY")
             message(FATAL_ERROR "given library '${ifl}' is NOT an interface library!")
             return()
          endif()
          get_target_property(_INC_DIRS ${tgt} INCLUDE_DIRECTORIES)
          get_target_property(_LNK_LIBS ${tgt} LINK_LIBRARIES)
          get_target_property(_IF_INCS  ${ifl} INTERFACE_INCLUDE_DIRECTORIES)
          get_target_property(_IF_LIBS  ${ifl} INTERFACE_LINK_LIBRARIES)
          if(_IF_INCS AND _INC_DIRS)
             set_target_properties(${tgt} PROPERTIES
                                   INCLUDE_DIRECTORIES "${_INC_DIRS};${_IF_INCS}"
                                   )
          elseif(_IF_INCS)
             set_target_properties(${tgt} PROPERTIES
                                   INCLUDE_DIRECTORIES "${_IF_INCS}"
                                   )
          endif()
          if(_IF_LIBS AND _LNK_LIBS)
             set_target_properties(${tgt} PROPERTIES
                                   LINK_LIBRARIES "${_LNK_LIBS};${_IF_LIBS}"
                                   )
          else()
             set_target_properties(${tgt} PROPERTIES
                                   LINK_LIBRARIES "${_IF_LIBS}"
                                   )
          endif()
          unset(_IF_INCS-NOTFOUND)
          unset(_INC_DIRS)
          unset(_LNK_LIBS)
          unset(_IF_INCS)
          unset(_IF_LIBS)
  endforeach()
  get_target_property(_LIBS ${tgt} LINK_LIBRARIES)
  list(REMOVE_DUPLICATES _LIBS)
  set_target_properties(${tgt} PROPERTIES
                        LINK_LIBRARIES "${_LIBS}"
                        )
endfunction(use_interface_libraries)

