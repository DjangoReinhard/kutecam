/* 
 * **************************************************************************
 * 
 *  file:       subop3dface.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    16.6.2022 by Django Reinhard
 *  copyright:  (c) 2022 Django Reinhard -  all rights reserved
 * 
 *  This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 2 of the License, or 
 *  (at your option) any later version. 
 *   
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 *   
 *  You should have received a copy of the GNU General Public License 
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * **************************************************************************
 */
#include "subop3dface.h"
#include "ui_opSub.h"
#include "ui_mainwindow.h"
#include "core.h"
#include "kuteCAM.h"
#include "occtviewer.h"
#include "operationlistmodel.h"
#include "pathbuilder.h"
#include "util3d.h"
#include <BRep_Tool.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <QAction>
#include <QDebug>


SubOP3DFace::SubOP3DFace(OperationListModel* olm, TargetDefListModel* tdModel, PathBuilder* pb, QWidget* parent)
 : OperationSubPage(olm, tdModel, pb, parent) {
  connect(Core().uiMainWin()->action3DFaceNew, &QAction::triggered, this, &SubOP3DFace::createOP);
  }


void SubOP3DFace::createOP() {
  int mx = olm->rowCount();

  OperationSubPage::createOP(mx, QString(tr("Notch #%1")).arg(mx), NotchOperation);
  connectSignals();
  }


void SubOP3DFace::processSelection() {
  std::vector<TopoDS_Shape> selection = Core().view3D()->selection();

  if (selection.size() > 1) {
     }
  }


void SubOP3DFace::processTargets() {

  }


void SubOP3DFace::genFinishingToolPath() {

  }


void SubOP3DFace::genRoughingToolPath() {

  }
