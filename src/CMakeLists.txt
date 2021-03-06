cmake_minimum_required(VERSION 3.16)
#=================================   APP   ===================================
project(kuteCAM LANGUAGES CXX)
#=============================================================================
include(Interface)
add_executable(kuteCAM "")
set(RESOURCE_FILES
    ../kuteCAM.qrc
    )
set(SOURCE_FILES
    GCodeEditor.ui
    cfgGCode.ui
    cfgMat.ui
    cfgVise.ui
    clip.ui
    mainwindow.ui
    misc.ui
    opSub.ui
    opSim.ui
    operations.ui
    preview.ui
    setup.ui
    status.ui
    tCutParm.ui
    tDim.ui
    tool.ui
    #-------------
    Geom_HelixData.cpp
    HHelixCurveAdaptor.cpp
    HelixCurveAdaptor.cpp
    HelixCurveAdaptor_CylinderEvaluator.cpp
    aboutdialog.cpp
    applicationwindow.cpp
    cctargetdefinition.cpp
    cfggcode.cpp
    cfgmaterial.cpp
    cfgvise.cpp
    clipdialog.cpp
    configpage.cpp
    contourtargetdefinition.cpp
    core.cpp
    cutparamlistmodel.cpp
    cutparmtooleditor.cpp
    dimtooleditor.cpp
    drilltargetdefinition.cpp
    editorpage.cpp
    gcodeeditor.cpp
    gcodehighlighter.cpp
    gcodewriter.cpp
    geomnodemodel.cpp
    gocircle.cpp
    gocontour.cpp
    goline.cpp
    gopocket.cpp
    graphicobject.cpp
    kernel.cpp
    kuteCAM.cpp
    main.cpp
    mainwindow.cpp
    occtviewer.cpp
    operation.cpp
    operationlistmodel.cpp
    operationspage.cpp
    operationsubpage.cpp
    pathbuilder.cpp
    pluginlistmodel.cpp
    preview3d.cpp
    projectfile.cpp
    selectionhandler.cpp
    setuppage.cpp
    shapelistmodel.cpp
    stringlistmodel.cpp
    subopclampingplug.cpp
    subopcontour.cpp
    subopdrill.cpp
    subopsweep.cpp
    subsimulation.cpp
    sweeptargetdefinition.cpp
    targetdefinition.cpp
    targetdeflistmodel.cpp
    tdfactory.cpp
    tooleditor.cpp
    toollistmodel.cpp
    util3d.cpp
    viseentry.cpp
    viselistmodel.cpp
    work.cpp
    workstep.cpp
    wsarc.cpp
    wscycle.cpp
    wsfactory.cpp
    wsstraightmove.cpp
    wstraverse.cpp
    xmltoolreader.cpp
    xmltoolwriter.cpp
    )
use_interface_libraries(kuteCAM
                        IFQt
                        IFOpenCASCADE
                        IFStandard
                        )
target_sources(kuteCAM PRIVATE
               ${SOURCE_FILES}
               ${RESOURCE_FILES}
               ${kuteCAM_QM_FILES}
               )
target_link_libraries(kuteCAM
                      PRIVATE KCPPLib
                      )
if(UNIX)
  target_link_libraries(kuteCAM PRIVATE
                        Qt5::X11Extras
                        ${X11_LIBRARIES}
                        )
endif()
target_include_directories(kuteCAM
                           PRIVATE ${CMAKE_SOURCE_DIR}
                           )
add_subdirectory(pp)
add_subdirectory(PPLib)
