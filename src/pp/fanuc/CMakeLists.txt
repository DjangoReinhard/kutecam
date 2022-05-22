cmake_minimum_required(VERSION 3.16)
#------------------------------   PLUGIN   -----------------------------------
project(ppFanuc LANGUAGES CXX)
#-----------------------------------------------------------------------------
include(Interface)
add_library(ppFanuc MODULE
    ppfanuc.cpp
    )
target_link_libraries(ppFanuc
                      PRIVATE KCPPLib
                      )
use_interface_libraries(ppFanuc
                        IFQt
                        IFOpenCASCADE
                        IFStandard
                        )

