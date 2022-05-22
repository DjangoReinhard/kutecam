cmake_minimum_required(VERSION 3.16)
#------------------------------   PLUGIN   -----------------------------------
project(ppSinumeric_840D LANGUAGES CXX)
#-----------------------------------------------------------------------------
include(Interface)
add_library(ppSinumeric_840D MODULE
    ppSinumeric840D.cpp
    )
target_link_libraries(ppSinumeric_840D
                      PRIVATE KCPPLib
                      )
use_interface_libraries(ppSinumeric_840D
                        IFQt
                        IFOpenCASCADE
                        IFStandard
                        )

