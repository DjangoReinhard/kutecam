cmake_minimum_required(VERSION 3.16)
#------------------------------   PLUGIN   -----------------------------------
project(ppFanuc LANGUAGES CXX)
#-----------------------------------------------------------------------------
include(Interface)
add_library(ppFanuc MODULE
    ppfanuc.cpp
    )
target_include_directories(ppFanuc
                           PRIVATE ../include
                           )
#target_link_libraries(ppFanuc
#                      PRIVATE FVBaseLib
#                      PRIVATE FVguiLib
#                      )
use_interface_libraries(ppFanuc
                        IFQt
                        IFOpenCASCADE
                        IFStandard
                        )

