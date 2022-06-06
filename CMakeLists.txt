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
             Core
             Gui
             LinguistTools
             Sql
             UiTools
             Widgets
             Xml
             )
if(UNIX)
  find_package(Qt5 COMPONENTS REQUIRED X11Extras)
  find_package(X11 REQUIRED)
endif()
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
set(TS_FILES
   i18n/kuteCAM_en.ts
   i18n/kuteCAM_de.ts
   )

qt5_add_translation(kuteCAM_QM_FILES ${kuteCAM_TS_FILES})
add_custom_target(translations DEPENDS ${kuteCAM_TS_FILES})
if(NOT QT_TRANSLATIONS_DIR)
  get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
  execute_process(COMMAND ${QT5_QMAKE_EXECUTABLE} -query QT_INSTALL_TRANSLATIONS
                  OUTPUT_VARIABLE qt_translations_dir
                  OUTPUT_STRIP_TRAILING_WHITESPACE
                  )
  file(TO_CMAKE_PATH "${qt_translations_dir}" qt_translations_dir)
  set(QT_TRANSLATIONS_DIR ${qt_translations_dir}
      CACHE PATH "The location of the Qt translations" FORCE
      )
endif()
add_custom_command(TARGET translations PRE_BUILD
                   COMMAND lupdate -silent ${CMAKE_CURRENT_SOURCE_DIR}/src -ts ${CMAKE_CURRENT_SOURCE_DIR}/i18n/kuteCAM_de.ts ${CMAKE_CURRENT_SOURCE_DIR}/i18n/kuteCAM_en.ts
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
                   )
add_custom_command(TARGET translations PRE_BUILD
                   COMMAND lrelease -compress -removeidentical ${CMAKE_CURRENT_SOURCE_DIR}/i18n/kuteCAM_de.ts -qm ${CMAKE_CURRENT_SOURCE_DIR}/i18n/kuteCAM_de.qm
                   )
add_custom_command(TARGET translations PRE_BUILD
                   COMMAND lrelease -compress -removeidentical ${CMAKE_CURRENT_SOURCE_DIR}/i18n/kuteCAM_en.ts -qm ${CMAKE_CURRENT_SOURCE_DIR}/i18n/kuteCAM_en.qm
                   )

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/i18n ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/i18n ${TS_FILES})
endif()
add_subdirectory(src)
add_dependencies(kuteCAM translations)

