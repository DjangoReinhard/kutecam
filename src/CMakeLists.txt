cmake_minimum_required(VERSION 3.16)
#=================================   APP   ===================================
project(kuteCAM LANGUAGES CXX)
#=============================================================================
include(Interface)
add_executable(kuteCAM
    applicationwindow.cpp
    configpage.cpp
    contourtargetdefinition.cpp
    core.cpp
    cuttingparameters.cpp
    cutparamlistmodel.cpp
    drilltargetdefinition.cpp
    gcodewriter.cpp
    geomlistmodel.cpp
    gocircle.cpp
    gocontour.cpp
    goline.cpp
    gopocket.cpp
    graphicobject.cpp
    kernel.cpp
    main.cpp
    mainwindow.cpp
    occtviewer.cpp
    operation.cpp
    operationlistmodel.cpp
    operationspage.cpp
    operationsubpage.cpp
    pathbuilder.cpp
    postprocessor.cpp
    ppfanuc.cpp
    preview3d.cpp
    projectfile.cpp
    selectionhandler.cpp
    setuppage.cpp
    shapelistmodel.cpp
    kuteCAM.cpp
    stringlistmodel.cpp
    subopcontour.cpp
    subopdrill.cpp
    subopsweep.cpp
    sweeptargetdefinition.cpp
    targetdefinition.cpp
    targetdeflistmodel.cpp
    tdfactory.cpp
    tooleditor.cpp
    toolentry.cpp
    toollistmodel.cpp
    toolpage.cpp
    util3d.cpp
    viseentry.cpp
    viselistmodel.cpp
    work.cpp
    workstep.cpp
    wpcutter.cpp
    wsarc.cpp
    wscycle.cpp
    wsfactory.cpp
    wsstraightmove.cpp
    wstraverse.cpp
    xmltoolreader.cpp
    xmltoolwriter.cpp
    ../kuteCAM.qrc
    )
set(TS_FILES
    kuteCAM.ts
    )
use_interface_libraries(kuteCAM
                        IFQt
                        IFOpenCASCADE
                        IFStandard
                        )
target_include_directories(kuteCAM
                           PRIVATE ${CMAKE_SOURCE_DIR}
                           )
if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

