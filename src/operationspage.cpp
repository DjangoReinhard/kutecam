/* 
 * **************************************************************************
 * 
 *  file:       operationspage.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    23.4.2022 by Django Reinhard
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
#include "operationspage.h"
#include "ui_operations.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "core.h"
#include "drilltargetdefinition.h"
#include "gcodewriter.h"
#include "geomnodemodel.h"
#include "HelixCurveAdaptor.h"
#include "occtviewer.h"
#include "operation.h"
#include "operationlistmodel.h"
#include "operationsubpage.h"
#include "pathbuilder.h"
#include "pathbuilderutil.h"
#include "subop3dface.h"
#include "subopcontour.h"
#include "subopclampingplug.h"
#include "subopdrill.h"
#include "subopnotch.h"
#include "subsimulation.h"
#include "subopsweep.h"
#include "sweeptargetdefinition.h"
#include "postprocessor.h"
#include "projectfile.h"
#include "targetdefinition.h"
#include "targetdeflistmodel.h"
#include "toolentry.h"
#include "toollistmodel.h"
#include "util3d.h"
#include "work.h"
#include "kuteCAM.h"

#include <BRep_Tool.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomAbs_CurveType.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <gp_Vec.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <TopoDS_Iterator.hxx>

#include <QAction>
#include <QFileDialog>
#include <QDir>
#include <QKeyEvent>
#include <QListView>
#include <QMessageBox>
#include <QStackedLayout>
#include <QStringListModel>
#include <QThread>
#include <QVector3D>
#include <QDebug>


OperationsPage::OperationsPage(QWidget *parent)
 : ApplicationWindow(parent)
 , ui(new Ui::OperationsPage)
 , olm(new OperationListModel)
 , currentOperation(nullptr)
 , opStack(new QStackedLayout())
 , infoModel(new GeomNodeModel())
 , subPage(nullptr)
 , tdModel(new TargetDefListModel(&dummy)) {
  ui->setupUi(this);  
  ui->Operation->setLayout(opStack);
  ui->lstOperations->setModel(olm);
  ui->geomTree->setModel(infoModel);
  connect(Core().uiMainWin()->actionToolPath, &QAction::triggered, this, &OperationsPage::toolPath);
  connect(Core().uiMainWin()->actionHideToolpath, &QAction::triggered, this, &OperationsPage::toggleToolpath);
  connect(Core().uiMainWin()->actionSelReprocess, &QAction::triggered, this, &OperationsPage::reSelect);
  connect(Core().uiMainWin()->actionGenerate_GCode, &QAction::triggered, this, &OperationsPage::genGCode);
  connect(Core().uiMainWin()->actionSimulate, &QAction::triggered, this, &OperationsPage::simulate);
  connect(Core().uiMainWin()->actionSelection2Horizontal, &QAction::triggered, this, &OperationsPage::sel2Horizontal);
  connect(Core().uiMainWin()->actionSelection2Vertical, &QAction::triggered, this, &OperationsPage::sel2Vertical);
  connect(ui->lstOperations->selectionModel(),  &QItemSelectionModel::selectionChanged, this, &OperationsPage::opSelected);
  connect(Core().view3D(), &OcctQtViewer::selectionChanged,  this, &OperationsPage::selectionChanged);
  connect(this,      &OperationsPage::raiseMessage, Core().mainWin(), &MainWindow::setStatusMessage);
  connect(infoModel, &GeomNodeModel::raiseMessage, Core().mainWin(), &MainWindow::setStatusMessage);
  connect(ui->spA,   &QDoubleSpinBox::valueChanged, this, &OperationsPage::rotate);
  connect(ui->spB,   &QDoubleSpinBox::valueChanged, this, &OperationsPage::rotate);
  connect(ui->spC,   &QDoubleSpinBox::valueChanged, this, &OperationsPage::rotate);
  connect(ui->dsCut, &QDoubleSpinBox::valueChanged, this, &OperationsPage::cutDepthChanged);
  connect(this, &OperationsPage::modelChanged, Core().mainWin(), &MainWindow::refresh);
  ui->lstOperations->installEventFilter(this);
  ui->lstTarget->setModel(tdModel);
  ui->lstTarget->installEventFilter(this);
  pathBuilder         = new PathBuilder(new PathBuilderUtil());
  pages["Drill"]      = new SubOPDrill(olm,   tdModel, pathBuilder);
  pages["Contour"]    = new SubOPContour(olm, tdModel, pathBuilder);
  pages["Notch"]      = new SubOPNotch(olm,   tdModel, pathBuilder);
  pages["3D-surf"]    = new SubOP3DFace(olm,  tdModel, pathBuilder);
  pages["Sweep"]      = new SubOPSweep(olm,   tdModel, pathBuilder);
  pages["ClampPlug"]  = new SubOPClampingPlug(olm, tdModel, pathBuilder);
  pages["Simulation"] = new SubSimulation(olm, tdModel, pathBuilder);

  for (QMap<QString, OperationSubPage*>::const_iterator i = pages.constBegin()
     ; i != pages.constEnd()
     ; ++i) {
    opStack->addWidget(i.value());
    connect(i.value(), &OperationSubPage::opCreated, this, &OperationsPage::addOperation);
    }  
  switch (Core().machineType()) {
    case 1:
         ui->spB->setEnabled(false);
         break;
    case 2:
         ui->spA->setEnabled(false);
         break;
    default:
         break;
    }
  }


void OperationsPage::addOperation(Operation* op) { // op stil not current operation
  if (op->kind() == ClampingPlugOP)  {
     op->setOperationA(0);
     op->setOperationB(0);
     op->setOperationC(0);
     ui->spA->setValue(0);
     ui->spB->setValue(0);
     ui->spC->setValue(0);
     }
  else {
     op->setOperationA(ui->spA->value());
     op->setOperationB(ui->spB->value());
     op->setOperationC(ui->spC->value());
     }
  olm->append(op);
  int         total = ui->lstOperations->model()->rowCount();
  QModelIndex mi    = olm->index(total - 1);

  qDebug() << "set current (operation) index to" << mi.row();

  ui->lstOperations->setCurrentIndex(mi);
  }


void OperationsPage::calcRotation4(const gp_Dir &n, double& aA, double& aB, double& aC) {
  double angles[3]   = {0};
  int    machineType = Core().machineType();
  gp_Pnt norm(n.X(), n.Y(), 0); // start with C-axis (projection to XY-plane)
  double a0  = atan2(norm.X(), norm.Y());
  double a1  = machineType == 2 ? atan2(1, 0) : atan2(0, 1);
  double len = norm.Distance({0, 0, 0});

  // angles[0] rotates the model around C-axis until norm-vector
  // points towards A- or B-axis (depending on machine type
  // z was set to 0 on projection, x or y is set to 0 by rotation
  angles[2] = a0 - a1;

  if (machineType == 2) { // B-axis is main rotation axis
     norm = gp_Pnt(len, 0, n.Z());
     a0 = atan2(norm.Z(), norm.X());
     a1 = atan2(1, 0);
     angles[1] = a0 - a1;
     }
  else {  // treat A-axis as main rotation axis
     norm = gp_Pnt(0, len, n.Z());
     a0 = atan2(norm.Z(), norm.Y());
     a1 = atan2(1, 0);
     angles[0] = a1 - a0;
     }
  aA = angles[0];
  aB = angles[1];
  aC = angles[2];
  }


void OperationsPage::clear() {
  olm->clear();
  }


void OperationsPage::cutDepthChanged(double d) {
  if (currentOperation) currentOperation->setWaterlineDepth(d);
  if (opStack->currentWidget() == pages["Contour"]) {
     SubOPContour* scp = static_cast<SubOPContour*>(pages["Contour"]);

     scp->updateCut(d);
     }
  }


void OperationsPage::closeEvent(QCloseEvent* e) {
  if (olm->rowCount()) saveOperations();
  }


bool OperationsPage::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
     QKeyEvent *evt = static_cast<QKeyEvent *>(event);

     switch (evt->key()) {
       case Qt::Key_Up:
            if (evt->modifiers() == Qt::CTRL) {
               QModelIndex mi = ui->lstOperations->currentIndex();

               olm->moveUp(mi);
               mi = olm->index(mi.row() - 1, 0);
               ui->lstOperations->setCurrentIndex(mi);

               return true;
               }
            break;
       case Qt::Key_Down:
            if (evt->modifiers() == Qt::CTRL) {
               QModelIndex mi = ui->lstOperations->currentIndex();

               olm->moveDown(mi);
               mi = olm->index(mi.row() + 1, 0);
               ui->lstOperations->setCurrentIndex(mi);

               return true;
               }
            break;
       case Qt::Key_Delete:
            if (obj == ui->lstTarget) {
               QMessageBox::StandardButton reply;

               reply = QMessageBox::question(this
                                           , tr("Delete Target Definition?")
                                           , tr("<p>You're going to delete the selected target "
                                                "definition. This operation can not be undone.</p>"
                                                "<p>Proceed?</p>")
                                           , QMessageBox::Yes | QMessageBox::Cancel);
               if (reply == QMessageBox::Yes) {
                  qDebug() << "DELETE on target list pressed ...";
                  qDebug() << "\tshould delete target definition #" << ui->lstTarget->currentIndex();

                  tdModel->removeRow(ui->lstTarget->currentIndex().row());

                  return true;
                  }
               }
            else if (obj == ui->lstOperations) {
               QMessageBox::StandardButton reply;

               reply = QMessageBox::question(this
                                           , tr("Delete Operation?")
                                           , tr("<p>You're going to delete the selected Operation.</p>"
                                                "<p>Proceed?</p>")
                                           , QMessageBox::Yes | QMessageBox::Cancel);
               if (reply == QMessageBox::Yes) {
                  qDebug() << "DELETE on operations list pressed ...";
                  qDebug() << "\tshould delete operation #" << ui->lstOperations->currentIndex();

                  //TODO: handle currentOperation and reselection!
//                  Operation* someOP = olm->operation(ui->lstOperations->currentIndex().row());
                  olm->removeRow(ui->lstOperations->currentIndex().row());
                  ProjectFile* pf = Core().projectFile();

                  pf->remove("Operations");

                  return true;
                  }
               }
            break;
       }
     }
  return false;
  }


void OperationsPage::genGCode() {
  PostProcessor* pp = Core().loadPostProcessor(Core().postProcessor());
  QString        xtension = pp->getFileExtension();
  QString        fileName;
  QString        filePattern = QString(tr("GCode Files (*.%1)")).arg(xtension);
  QFileDialog    dialog(this
                      , tr("QFileDialog::getSaveFileName()")
                      , kute::BasePath
                      , filePattern);

  dialog.setSupportedSchemes(QStringList(QStringLiteral("file")));
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted) {
     qDebug() << "Oups - file-dialog aborted!";
     return;
     }
  fileName = dialog.selectedUrls().value(0).toLocalFile();
  if (!fileName.endsWith(QString(".%1").arg(xtension)))
     fileName += "." + xtension;

  qDebug() << "save gcode to:" << fileName;

  GCodeWriter gcw(pp);
  Bnd_Box     wpBounds = Core().workData()->workPiece->BoundingBox();
  bool        genTC    = Core().isSepWithToolChange();

  if (Core().isAllInOneOperation()) {
     gcw.processAllInOne(fileName,  wpBounds, olm->operations());
     emit fileGenerated(fileName);
     }
  else
     gcw.processSingleOPs(fileName, wpBounds, olm->operations(), genTC);
  }


void OperationsPage::handleMachineType(int machineType) {
  Core().setMachineType(machineType);

  qDebug() << "OP::machine-type: " << machineType;
  switch (machineType) {
    case 1:
         ui->spA->setEnabled(true);
         ui->spB->setEnabled(false);
         break;
    case 2:
         ui->spA->setEnabled(false);
         ui->spB->setEnabled(true);
         break;
    case 3:
         ui->spA->setEnabled(true);
         ui->spB->setEnabled(true);
         break;
    }
  }


void OperationsPage::loadOperation(Operation* op) { // don't touch old operation any more!
  if (!op) return;
  if (currentOperation) {
     if (currentOperation->toolPaths.size()) Core().view3D()->removeShapes(currentOperation->toolPaths);
     if (currentOperation->cShapes.size())   Core().view3D()->removeShapes(currentOperation->cShapes);
     }
  double dA = op->operationA();
  double dB = op->operationB();
  double dC = op->operationC();

  ui->spA->setValue(dA);
  ui->spB->setValue(dB);
  ui->spC->setValue(dC);
  Core().view3D()->rotate(kute::deg2rad(dA), kute::deg2rad(dB), kute::deg2rad(dC));

  switch (op->kind()) {
    case ContourOperation: subPage = pages["Contour"];   break;
    case DrillOperation:   subPage = pages["Drill"];     break;
    case NotchOperation:   subPage = pages["Notch"];     break;
    case SweepOperation:   subPage = pages["Sweep"];     break;
    case ClampingPlugOP:   subPage = pages["ClampPlug"]; break;
    }
  opStack->setCurrentWidget(subPage);

  currentOperation = op;
  subPage->loadOP(currentOperation);

  ui->dsCut->setValue(op->waterlineDepth());
  if (currentOperation && currentOperation->workSteps().size())
     subPage->showToolPath(currentOperation);
  }


void OperationsPage::loadProject(ProjectFile *pf) {
  if (!pf) return;
  std::vector<Operation*> opList = Core().loadOperations(pf);

  olm->setData(opList);
  if (opList.size()) ui->lstOperations->selectionModel()->select(olm->index(0, 0), QItemSelectionModel::SelectCurrent);
  }


void OperationsPage::opSelected(const QItemSelection &selected, const QItemSelection &deselected) {
  QModelIndexList il    = selected.indexes();
  QModelIndex     mi    = il.at(0);
  Operation*      nxtOP = olm->operation(mi.row());

  qDebug() << "current operation changed: #" << mi.row();

  loadOperation(nxtOP);
  }


void OperationsPage::reSelect() {
  if (!subPage) return;
  if (currentOperation) {
     currentOperation->setOperationA(ui->spA->value());
     currentOperation->setOperationB(ui->spB->value());
     currentOperation->setOperationC(ui->spC->value());
     }
  subPage->fixit();
  currentOperation->targets.clear();
  subPage->processSelection();
  if (currentOperation && currentOperation->cShapes.size())
     Core().view3D()->showShapes(currentOperation->cShapes, false);
  }


void OperationsPage::rotate() {
  double dA = ui->spA->value();
  double dB = ui->spB->value();
  double dC = ui->spC->value();

  if (currentOperation) Core().view3D()->removeShapes(currentOperation->toolPaths);
  Core().view3D()->rotate(kute::deg2rad(dA), kute::deg2rad(dB), kute::deg2rad(dC));
  }


void OperationsPage::saveOperations() {
  if (currentOperation) {
     ProjectFile* pf = Core().projectFile();

     pf->beginGroup("Work");
     pf->beginWriteArray("Operations");
     for (int i=0; i < olm->rowCount(); ++i) {
         Operation* op = olm->operation(i);

         pf->setArrayIndex(i);
         op->store(pf->settings());
         }
     pf->endArray();
     pf->endGroup();
     }
  }


void OperationsPage::sel2Horizontal() {
//  if (!Core().view3D()->selection().size()) return;
//  TopoDS_Shape            selectedShape = Core().view3D()->selection().at(0);
//  Handle(Geom_Surface)    selectedFace  = BRep_Tool::Surface(TopoDS::Face(selectedShape));
//  GeomAdaptor_Surface     selectedSurface(selectedFace);
//  gp_Pln                  pln = selectedSurface.Plane();
//  gp_Dir                  dir = Core().helper3D()->deburr(pln.Axis().Direction());
//  gp_Vec                  v;
//  double                  a;

//  qDebug() << "selected face has normal:" << dir.X() << " / " << dir.Y() << " / " << dir.Z();


//  if (!kute::isEqual(dir.X(), 0)) {
//     v = gp_Vec(dir.X(), 0, 0);
//     a = v.Angle({0, 0, 1});
//     ui->spA->setValue(kute::rad2deg(a));
//    }
//  if (!kute::isEqual(dir.Y(), 0)) {
//     v = gp_Vec(0, dir.Y(), 0);
//     a = v.Angle({0, 0, 1});
//     ui->spB->setValue(kute::rad2deg(a));
//     }
//  if (!kute::isEqual(dir.Z(), 1)) {
//     v = gp_Vec(0, 0, dir.Z());
//     a = v.Angle({0, 0, 1});
//     ui->spC->setValue(kute::rad2deg(a));
//     }
  }


void OperationsPage::sel2Vertical() {
//  if (!Core().view3D()->selection().size()) return;
//  TopoDS_Shape            selectedShape = Core().view3D()->selection().at(0);
//  Handle(Geom_Surface)    selectedFace  = BRep_Tool::Surface(TopoDS::Face(selectedShape));
//  GeomAdaptor_Surface     selectedSurface(selectedFace);
//  gp_Pln                  pln = selectedSurface.Plane();
//  gp_Dir                  dir = Core().helper3D()->deburr(pln.Axis().Direction());

//  qDebug() << "selected face has normal:" << dir.X() << " / " << dir.Y() << " / " << dir.Z();

//  gp_Vec                  v(dir.X(), 0, 0);
//  double                  a = v.Angle({1, 0, 0});

//  ui->spA->setValue(kute::rad2deg(a));

//  v = gp_Vec(0, dir.Y(), 0);
//  a = v.Angle({1, 0, 0});
//  ui->spB->setValue(kute::rad2deg(a));

//  v = gp_Vec(0, 0, dir.Z());
//  a = v.Angle({1, 0, 0});
//  ui->spC->setValue(kute::rad2deg(a));
  }


void OperationsPage::simulate() {       
  if (currentOperation->toolPaths.size()) {
     subPage = pages["Simulation"];
     opStack->setCurrentWidget(subPage);

     subPage->loadOP(currentOperation);
     subPage->toolPath();
     }
  }


void OperationsPage::shapeSelected(const TopoDS_Shape &shape) {
  qDebug() << "shape selection changed ...";
  Handle(AIS_Shape) tmp = new AIS_Shape(shape);
  const Bnd_Box     bb  = tmp->BoundingBox();

  if (currentOperation)
     currentOperation->shBounds = bb;
  emit raiseMessage(QString("%1 <-> %2").arg(bb.CornerMax().Z())
                                        .arg(bb.CornerMin().Z()));
  }


void OperationsPage::rotateIfFace(const std::vector<TopoDS_Shape> &selection) {
  if (selection.size() != 1) return;
  auto s = selection.at(0);

  if (s.ShapeType() == TopAbs_FACE) {
     qDebug() << "selection is face ...";

     Handle(Geom_Surface) selectedFace = BRep_Tool::Surface(TopoDS::Face(s));
     GeomNode* node = nullptr;

     if (selectedFace->IsKind(STANDARD_TYPE(Geom_Plane))) {
        GeomAdaptor_Surface selectedSurface(selectedFace);
        gp_Pln              pln = selectedSurface.Plane();
        gp_Pnt              pos = Core().helper3D()->deburr(pln.Location());
        gp_Dir              dir = Core().helper3D()->deburr(pln.Axis().Direction());
        double              aA = 0, aB = 0, aC = 0;

        //TODO: respect actual angle settings
        calcRotation4(dir, aA, aB, aC);

        ui->spA->setValue(kute::rad2deg(aA));
        ui->spB->setValue(kute::rad2deg(aB));
        ui->spC->setValue(kute::rad2deg(aC));
        Core().view3D()->rotate(aA, aB, aC);
        }
     }
  }


void OperationsPage::selectionChanged() {
  std::vector<TopoDS_Shape> selection = Core().view3D()->selection();

  if (Core().autoRotateSelection()) rotateIfFace(selection);
  infoModel->replaceData(selection);
  ui->geomTree->expandAll();
  }


void OperationsPage::toggleToolpath() {
  if (Core().uiMainWin()->actionHideToolpath->isChecked()) {
     if (currentOperation->toolPaths.size()) {
        Core().view3D()->removeShapes(currentOperation->toolPaths);
        }
     if (currentOperation->cShapes.size()) {
        Core().view3D()->removeShapes(currentOperation->cShapes);
        }
     }
  else {
     if (currentOperation->toolPaths.size()) {
        Core().view3D()->showShapes(currentOperation->toolPaths /*, false */);
        }
     if (currentOperation->cShapes.size()) {
        Core().view3D()->showShapes(currentOperation->cShapes /*, false */);
        }
     }
  Core().view3D()->refresh();
  }


void OperationsPage::toolPath() {
  if (!currentOperation) return;
  if (currentOperation->toolPaths.size()) {
     Core().view3D()->removeShapes(currentOperation->toolPaths);
     currentOperation->toolPaths.clear();
     }
  if (currentOperation->cShapes.size()) {
     Core().view3D()->removeShapes(currentOperation->cShapes);
     currentOperation->cShapes.clear();
     }
  if (currentOperation->workSteps().size()) currentOperation->workSteps().clear();
  if (subPage) {
     Core().uiMainWin()->actionHideToolpath->setChecked(false);
     subPage->fixit();
     subPage->toolPath();
     }
  }
