cmake_minimum_required(VERSION 3.16)
#--------------------------------   LIB   ------------------------------------
project(PPLib LANGUAGES CXX)
#-----------------------------------------------------------------------------
include(Interface)
add_library(KCPPLib STATIC
    abstractpostprocessor.cpp
    cuttingparameters.cpp
    dinpostprocessor.cpp
    toolentry.cpp
    )
set(TS_FILES
    i18n/PPLib_de_DE.ts
    )
target_include_directories(KCPPLib
                           PUBLIC ${CMAKE_SOURCE_DIR}
                           )
use_interface_libraries(KCPPLib
                        IFQt
                        IFOpenCASCADE
                        IFStandard
                        )
if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()


