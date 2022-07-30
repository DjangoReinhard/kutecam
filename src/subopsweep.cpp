/* 
 * **************************************************************************
 * 
 *  file:       subopsweep.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    24.4.2022 by Django Reinhard
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
#include "subopsweep.h"
#include "gocontour.h"
#include "gopocket.h"
#include "ui_opSub.h"
#include "ui_mainwindow.h"
#include "operationlistmodel.h"
#include "occtviewer.h"
#include "core.h"
#include "cuttingparameters.h"
#include "kuteCAM.h"
#include "pathbuilder.h"
#include "selectionhandler.h"
#include "sweeptargetdefinition.h"
#include "targetdefinition.h"
#include "targetdeflistmodel.h"
#include "toollistmodel.h"
#include "toolentry.h"
#include "util3d.h"
#include "work.h"
#include "wsarc.h"
#include "wstraverse.h"
#include "wsstraightmove.h"

#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Surface.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <TopoDS_Iterator.hxx>

#include <QAction>
#include <QStringListModel>
#include <QDebug>


SubOPSweep::SubOPSweep(OperationListModel* olm, TargetDefListModel* tdModel, PathBuilder* pb, QWidget *parent)
 : OperationSubPage(olm, tdModel, pb, parent) {
  ui->lCycle->setVisible(false);
  ui->cbCycle->setVisible(false);
  ui->lRetract->setVisible(false);
  ui->spRetract->setVisible(false);
  ui->spDwell->setVisible(false);
  connect(Core().uiMainWin()->actionSweepNew, &QAction::triggered, this, &SubOPSweep::createOP);
  }


void SubOPSweep::createOP() { // TODO: save possible changes to old operation ?
  int mx = olm->rowCount();

  OperationSubPage::createOP(mx, QString(tr("Sweep #%1")).arg(mx), SweepOperation);
  connectSignals();
  }


void SubOPSweep::processSelection() {
  if (curOP->cShapes.size()) {
     Core().view3D()->removeShapes(curOP->cShapes);
     curOP->cShapes.clear();
     }
  tdModel->clear();
  if (!Core().view3D()->selection().size()) return;
  SweepTargetDefinition* std   = nullptr;
  Handle(AIS_Shape)      baseFace = Core().helper3D()->fixRotation(Core().view3D()->baseFace()->Shape()
                                                                 , curOP->operationA()
                                                                 , curOP->operationB()
                                                                 , curOP->operationC());
  gp_Vec            baseNormal = Core().helper3D()->deburr(Core().helper3D()->normalOfFace(baseFace->Shape()));
  GOContour*        contour = nullptr;
  Handle(AIS_Shape) cutPart;
  Bnd_Box           bbCP;

  if (Core().view3D()->selection().size() > 1) {
     // multiple selections should be some vertical faces or
     // some edges that will lead to a vertical cutface
     contour = Core().selectionHandler()->createContourFromSelection(curOP);
     if (!contour) return;
     contour->extendBy(50);
     TopoDS_Shape      cutWire = contour->toShape(-500)->Shape();
     Handle(AIS_Shape) aw      = new AIS_Shape(cutWire);
     gp_Vec            prismVec(0, 0, 1000);
     TopoDS_Shape      cuttingFace = BRepPrimAPI_MakePrism(cutWire, prismVec);
     Handle(AIS_Shape) aCF         = new AIS_Shape(cuttingFace);
     gp_Pnt            center;

     cutPart = Core().selectionHandler()->createCutPart(curOP->workPiece, cuttingFace, curOP, curOP->isOutside());
     bbCP    = cutPart->BoundingBox();
     aCF->SetColor(Quantity_NOC_CYAN);
     aw->SetColor(Quantity_NOC_ORANGE);
     aCF->SetTransparency(0.8);
     aw->SetWidth(3);

     center.SetX(bbCP.CornerMin().X() + (bbCP.CornerMax().X() - bbCP.CornerMin().X()) / 2);
     center.SetY(bbCP.CornerMin().Y() + (bbCP.CornerMax().Y() - bbCP.CornerMin().Y()) / 2);
     center.SetZ(bbCP.CornerMin().Z() + (bbCP.CornerMax().Z() - bbCP.CornerMin().Z()) / 2);
     if (contour->centerPoint().X() < (center.X() - 5))
        center.SetX(bbCP.CornerMin().X());
     else if (contour->centerPoint().X() > (center.X() + 5))
        center.SetX(bbCP.CornerMax().X());
     if (contour->centerPoint().Y() < (center.Y() - 5))
        center.SetY(bbCP.CornerMin().Y());
     else if (contour->centerPoint().Y() > (center.Y() + 5))
        center.SetY(bbCP.CornerMax().Y());
     if (!curOP->workPiece.IsNull()) {
        TopoDS_Shape master = BRepAlgoAPI_Common(contour->toWire(0), curOP->workPiece->Shape());
        Handle(AIS_Shape) asM = new AIS_Shape(master);

        qDebug() << "contour of selection: " << contour->toString();

        asM->SetColor(Quantity_NOC_ORANGE);
        asM->SetWidth(3);

        curOP->cShapes.push_back(asM);
        contour->setContour(master);
        }
     std = new SweepTargetDefinition(contour);
//     std->setBaseIsBorder(true);
     curOP->setVertical(true);
     curOP->cShapes.push_back(aw);
     curOP->cShapes.push_back(aCF);
     ui->spDepth->setValue(bbCP.CornerMin().Z());
     }
  else { // only one face selected
     TopoDS_Shape            selectedShape = Core().view3D()->selection().at(0);
     Handle(Geom_Surface)    selectedFace  = BRep_Tool::Surface(TopoDS::Face(selectedShape));
     Handle(AIS_Shape)       asTmp         = new AIS_Shape(selectedShape);
     Bnd_Box                 bbSel         = asTmp->BoundingBox(); bbSel.SetGap(0);
     GeomAdaptor_Surface     selectedSurface(selectedFace);
     gp_Pln                  pln = selectedSurface.Plane();
     BRepBuilderAPI_MakeFace mf(pln, -500, 500, -500, 500);

     // sweep handles plane faces only
     if (selectedFace->IsKind(STANDARD_TYPE(Geom_Plane))) {
        gp_Pnt pos = Core().helper3D()->deburr(pln.Location());
        gp_Dir dir = Core().helper3D()->deburr(pln.Axis().Direction());

        // vertical plane (limited by spDepth)
        if (kute::isEqual(dir.Z(), 0)) {
           cutPart = Core().selectionHandler()->createCutPart(curOP->workPiece, mf.Shape(), curOP, curOP->isOutside());
           bbCP    = cutPart->BoundingBox();
           contour = Core().selectionHandler()->createContourFromSelection(curOP);
           if (!contour) return;
           contour->extendBy(200);
           TopoDS_Shape cutWire = contour->toShape(-100)->Shape();

           std = new SweepTargetDefinition(contour);
           curOP->setVertical(true);
           tdModel->append(std);
           ui->spDepth->setValue(curOP->finalDepth());
           }
        // horizontal plane (possibly limited by base face)
        else if (kute::isEqual(dir.Z(), 1)) {
           TopoDS_Shape master = Core().selectionHandler()->createBaseContour(pos, dir, curOP);

           contour = new GOContour(Core().helper3D()->centerOf(bbSel));
           contour->setContour(master);
           curOP->setVertical(false);
           if (contour->size()) {
              TopoDS_Shape      cutWire = contour->toWire();
              Handle(AIS_Shape) aw      = new AIS_Shape(cutWire);
              gp_Vec            prismVec(baseNormal.X(), baseNormal.Y(), 0);
              TopoDS_Shape      cuttingFace = BRepPrimAPI_MakePrism(cutWire, prismVec);
              Handle(AIS_Shape) aCF         = new AIS_Shape(cuttingFace);

              cutPart = Core().selectionHandler()->createCutPart(curOP->workPiece, mf.Shape(), curOP, curOP->isOutside());
              bbCP    = cutPart->BoundingBox();
              aCF->SetColor(Quantity_NOC_CYAN);
              aw->SetColor(Quantity_NOC_ORANGE);
              aCF->SetTransparency(0.8);
              aw->SetWidth(3);

              if (!curOP->workPiece.IsNull()) {
                 TopoDS_Shape master = BRepAlgoAPI_Common(contour->toWire(), curOP->workPiece->Shape());
                 contour->setContour(master);
                 }
              std = new SweepTargetDefinition(pos, dir);
              std->setContour(contour);
              std->setBaseIsBorder(true);
              tdModel->append(std);
              curOP->cShapes.push_back(aw);
              curOP->cShapes.push_back(aCF);
              ui->spDepth->setValue(bbCP.CornerMin().Z());
              }
           else {
              cutPart = Core().selectionHandler()->createCutPart(curOP->workPiece, mf.Shape(), curOP, curOP->isOutside());
              cutPart->SetColor(Quantity_NOC_CYAN);
              cutPart->SetTransparency(0.8);
              curOP->cShapes.push_back(cutPart);

              std = new SweepTargetDefinition(pos, dir);
              }
           ui->spDepth->setValue(pos.Z());
           }
        else qDebug() << "ERROR: selected shape is not horizontal nor vertical!";
        }
     else qDebug() << "ERROR: selected shape is not plane!";
     }  
  bbCP = cutPart->BoundingBox();
  cutPart->SetColor(Quantity_NOC_CYAN);
  cutPart->SetTransparency(0.8);
  std->setBaseDir(baseNormal);
  std->setBaseDelim(baseFace->BoundingBox());
//  std->setZMin(bbCP.CornerMin().Z());
//  std->setZMax(bbCP.CornerMax().Z());
  curOP->setUpperZ(bbCP.CornerMax().Z());
  curOP->setLowerZ(std->pos().Z());
  tdModel->append(std);
  curOP->cShapes.push_back(cutPart);
  Core().view3D()->showShapes(curOP->cShapes);
  Core().view3D()->refresh();
  }


void SubOPSweep::processTargets() {
  if (!tdModel->rowCount()) return;
  //sweep operations shall have one target definition only!
  SweepTargetDefinition* std = static_cast<SweepTargetDefinition*>(tdModel->item(0));

  if (!curOP->isVertical()) {
     gp_Pln                  cutPlane(std->pos(), std->dir());
     BRepBuilderAPI_MakeFace mf(cutPlane, -500, 500, -500, 500);

     curOP->cutPart = Core().selectionHandler()->createCutPart(curOP->workPiece, mf.Shape(), curOP, curOP->isOutside());
     }
  else {
     if (std->contour()) {
        // cutWire is border, cutPart the part to remove
        qDebug() << "reloaded cut contour:" << std->contour()->toString();
        TopoDS_Shape cutWire = std->contour()->toShape(-100)->Shape();
        Handle(AIS_Shape) aw = new AIS_Shape(cutWire);
        gp_Vec       prismVec(0, 0, 500);
        TopoDS_Shape cuttingFace = BRepPrimAPI_MakePrism(cutWire, prismVec);
        Handle(AIS_Shape) aCF         = new AIS_Shape(cuttingFace);

        aCF->SetColor(Quantity_NOC_CYAN);
        aCF->SetTransparency(0.8);
        aw->SetColor(Quantity_NOC_ORANGE);
        aw->SetWidth(3);
        curOP->cShapes.push_back(aw);
        curOP->cShapes.push_back(aCF);
        curOP->cutPart = Core().selectionHandler()->createCutPart(curOP->workPiece, cuttingFace, curOP, curOP->isOutside());
        }
     else {
        gp_Pln                  cutPlane(std->pos(), std->dir());
        BRepBuilderAPI_MakeFace mf(cutPlane, -500, 500, -500, 500);

        curOP->cutPart = Core().selectionHandler()->createCutPart(curOP->workPiece, mf.Shape(), curOP, curOP->isOutside());
        }
     }
  if (!curOP->cutPart.IsNull()) {
     curOP->cutPart->SetColor(Quantity_NOC_CYAN);
     curOP->cutPart->SetTransparency(0.8);
     curOP->cShapes.push_back(curOP->cutPart);
//     Core().view3D()->showShapes(curOP->cShapes, false);
     }
//  Core().view3D()->refresh();
  }


void SubOPSweep::genFinishingToolPath() {

  }


void SubOPSweep::genRoughingToolPath() {
  if (!curOP->cutDepth()) return;
  processTargets();
  if (curOP->isVertical()) {
     qDebug() << "OP sweep - gonna create VERTICAL toolpath ...";
     curOP->workSteps() = pathBuilder()->genToolPath(curOP, curOP->cutPart, false);
     }
  else {
     if (Core().workData()->roundWorkPiece) {
        std::vector<Handle(AIS_Shape)> cutPlanes = createCutPlanes(curOP);
        bool againstFeed = ui->cbDir->currentIndex() == 1;

        qDebug() << "OP sweep - gonna create ROUND toolpath ...";

        //TODO: round toolpaths with external lead-in!
        curOP->workSteps() = pathBuilder()->genRoundToolpaths(curOP, cutPlanes);
        }
     else {
        if (curOP->targets.size() && curOP->targets.at(0)) {
           SweepTargetDefinition* std = static_cast<SweepTargetDefinition*>(curOP->targets.at(0));

           if (std->contour()) {
              curOP->workSteps() = pathBuilder()->genToolPath(curOP, curOP->cutPart, false);
              }
           }
        if (!curOP->workSteps().size()) {
           // uses already built cutPart!
           std::vector<Handle(AIS_Shape)> cutPlanes = createCutPlanes(curOP);

           qDebug() << "OP sweep - gonna create HORIZONTAL toolpath ...";

           pPathBuilder->createHorizontalToolpaths(curOP, cutPlanes);
           }
        }
     }
  showToolPath(curOP);
  }
