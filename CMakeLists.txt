cmake_minimum_required(VERSION 3.16)
##################################   TOP   ###################################
project(kuteCAM VERSION 0.1)
##############################################################################
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_MODULE_PATH /d/linux/cmake)
include(Interface RESULT_VARIABLE ConfigBase)
set(CMAKE_PREFIX_PATH "/opt/qt5.15/5.15.2/gcc_64" CACHE PATH "CMake search-path")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ";/usr/local/lib/cmake/opencascade")
set(OpenCASCADE_DIR "/usr/local/lib/cmake/opencascade" CACHE PATH "OpenCascade 7.6.0")
set(ENV{CMAKE_BUILD_PARALLEL_LEVEL} "4")
### ------ don't change settings below this line ---- ###
find_package(Qt5 REQUIRED COMPONENTS
             Gui
             Widgets
             Core
             Sql
             UiTools
             Xml
             LinguistTools
             )
find_package(OpenCASCADE COMPONENTS
             FoundationClasses
             ModelingData
             ModelingAlgorithms
             Visualization
             ApplicationFramework
             Draw
             PATHS ${OpenCASCADE_DIR} NO_DEFAULT_PATH
             )
add_library(IFQt          INTERFACE)
add_library(IFOpenCASCADE INTERFACE)
add_library(IFStandard    INTERFACE)
target_link_libraries(IFQt
                      INTERFACE Qt${QT_VERSION_MAJOR}::Gui
                      INTERFACE Qt${QT_VERSION_MAJOR}::GuiPrivate
                      INTERFACE Qt${QT_VERSION_MAJOR}::Widgets
                      INTERFACE Qt${QT_VERSION_MAJOR}::Core
                      INTERFACE Qt${QT_VERSION_MAJOR}::Sql
                      INTERFACE Qt${QT_VERSION_MAJOR}::UiTools
                      INTERFACE Qt${QT_VERSION_MAJOR}::Xml
                      )
target_include_directories(IFOpenCASCADE
                           INTERFACE ${OpenCASCADE_INCLUDE_DIR}
                           )
target_link_libraries(IFOpenCASCADE
                      INTERFACE TKBinL
                      INTERFACE TKBin
                      INTERFACE TKBinTObj
                      INTERFACE TKBinXCAF
                      INTERFACE TKBool
                      INTERFACE TKBO
                      INTERFACE TKBRep
                      INTERFACE TKCAF
                      INTERFACE TKCDF
                      INTERFACE TKDCAF
                      INTERFACE TKDFBrowser
                      INTERFACE TKDraw
                      INTERFACE TKernel
                      INTERFACE TKFeat
                      INTERFACE TKFillet
                      INTERFACE TKG2d
                      INTERFACE TKG3d
                      INTERFACE TKGeomAlgo
                      INTERFACE TKGeomBase
                      INTERFACE TKHLR
                      INTERFACE TKIGES
                      INTERFACE TKLCAF
                      INTERFACE TKMath
                      INTERFACE TKMesh
                      INTERFACE TKMeshVS
                      INTERFACE TKMessageModel
                      INTERFACE TKMessageView
                      INTERFACE TKOffset
                      INTERFACE TKOpenGl
                      INTERFACE TKOpenGlTest
                      INTERFACE TKPrim
                      INTERFACE TKQADraw
                      INTERFACE TKRWMesh
                      INTERFACE TKService
                      INTERFACE TKShapeView
                      INTERFACE TKShHealing
                      INTERFACE TKStdL
                      INTERFACE TKStd
                      INTERFACE TKSTEP209
                      INTERFACE TKSTEPAttr
                      INTERFACE TKSTEPBase
                      INTERFACE TKSTEP
                      INTERFACE TKSTL
                      INTERFACE TKTInspectorAPI
                      INTERFACE TKTInspector
                      INTERFACE TKTObjDRAW
                      INTERFACE TKTObj
                      INTERFACE TKToolsDraw
                      INTERFACE TKTopAlgo
                      INTERFACE TKTopTest
                      INTERFACE TKTreeModel
                      INTERFACE TKV3d
                      INTERFACE TKVCAF
                      INTERFACE TKViewerTest
                      INTERFACE TKView
                      INTERFACE TKVInspector
                      INTERFACE TKVRML
                      INTERFACE TKXCAF
                      INTERFACE TKXDEDRAW
                      INTERFACE TKXDEIGES
                      INTERFACE TKXDESTEP
                      INTERFACE TKXMesh
                      INTERFACE TKXmlL
                      INTERFACE TKXml
                      INTERFACE TKXmlTObj
                      INTERFACE TKXmlXCAF
                      INTERFACE TKXSBase
                      INTERFACE TKXSDRAW
                      )
target_link_libraries(IFStandard
                      INTERFACE m
                      INTERFACE stdc++
                      INTERFACE crypt
                      INTERFACE pthread
                      INTERFACE dl
                      INTERFACE util
                      )
add_subdirectory(src)
