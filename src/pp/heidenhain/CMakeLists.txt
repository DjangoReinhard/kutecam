cmake_minimum_required(VERSION 3.16)
#------------------------------   PLUGIN   -----------------------------------
project(ppHeidenhain LANGUAGES CXX)
#-----------------------------------------------------------------------------
include(Interface)
add_library(ppHeidenhain MODULE
    ppheidenhain.cpp
    )
target_include_directories(ppHeidenhain
                           PRIVATE ../include
                           )
#target_link_libraries(ppHeidenhain
#                      PRIVATE FVBaseLib
#                      PRIVATE FVguiLib
#                      )
use_interface_libraries(ppHeidenhain
                        IFQt
                        IFOpenCASCADE
                        IFStandard
                        )

