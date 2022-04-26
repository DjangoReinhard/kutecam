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
#include "occtviewer.h"
#include "operation.h"
#include "operationlistmodel.h"
#include "operationsubpage.h"
#include "subopcontour.h"
#include "subopdrill.h"
#include "subopsweep.h"
#include "sweeptargetdefinition.h"
#include "ppfanuc.h"
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
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <gp_Vec.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <TopoDS_Iterator.hxx>

#include <QAction>
#include <QFileDialog>
#include <QDir>
#include <QListView>
#include <QMessageBox>
#include <QStackedLayout>
#include <QStringListModel>
#include <QVector3D>
#include <QDebug>


OperationsPage::OperationsPage(QWidget *parent)
 : ApplicationWindow(parent)
 , ui(new Ui::OperationsPage)
 , olm(new OperationListModel)
 , currentOperation(nullptr)
 , opStack(new QStackedLayout())
 , subPage(nullptr)
 , tdModel(new TargetDefListModel(&dummy)) {
  ui->setupUi(this);  
  ui->Operation->setLayout(opStack);
  ui->lstOperations->setModel(olm);
  connect(Core().uiMainWin()->actionToolPath, &QAction::triggered, this, &OperationsPage::toolPath);
  connect(Core().uiMainWin()->actionSelReprocess, &QAction::triggered, this, &OperationsPage::reSelect);
  connect(Core().uiMainWin()->actionGenerate_GCode, &QAction::triggered, this, &OperationsPage::genGCode);
  connect(ui->lstOperations->selectionModel(),  &QItemSelectionModel::selectionChanged, this, &OperationsPage::opSelected);
  connect(Core().view3D(), &OcctQtViewer::shapeSelected,  this, &OperationsPage::shapeSelected);
  connect(this,      &OperationsPage::raiseMessage, Core().mainWin(), &MainWindow::setStatusMessage);
  connect(ui->spA,   &QDoubleSpinBox::valueChanged, this, &OperationsPage::rotate);
  connect(ui->spB,   &QDoubleSpinBox::valueChanged, this, &OperationsPage::rotate);
  connect(ui->spC,   &QDoubleSpinBox::valueChanged, this, &OperationsPage::rotate);
  connect(ui->dsCut, &QDoubleSpinBox::valueChanged, this, &OperationsPage::cutDepthChanged);
  connect(this, &OperationsPage::modelChanged, Core().mainWin(), &MainWindow::refresh);
  ui->lstOperations->installEventFilter(this);
  ui->lstTarget->setModel(tdModel);
  ui->lstTarget->installEventFilter(this);
  pages["Drill"]   = new SubOPDrill(olm,   tdModel);
  pages["Contour"] = new SubOPContour(olm, tdModel);
  pages["Sweep"]   = new SubOPSweep(olm,   tdModel);

  for (QMap<QString, OperationSubPage*>::const_iterator i = pages.constBegin()
     ; i != pages.constEnd()
     ; ++i) {
    opStack->addWidget(i.value());
    connect(i.value(), &OperationSubPage::opCreated, this, &OperationsPage::addOperation);
    }  
  }


void OperationsPage::addOperation(Operation* op) { // op stil not current operation
  op->setOperationA(ui->spA->value());
  op->setOperationB(ui->spB->value());
  op->setOperationC(ui->spC->value());
  olm->append(op);
  int         total = ui->lstOperations->model()->rowCount();
  QModelIndex mi    = olm->index(total - 1);

  qDebug() << "set current (operation) index to" << mi.row();

  ui->lstOperations->setCurrentIndex(mi);
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
  ProjectFile* pf = Core().projectFile();

  switch (currentOperation->kind()) {
    case ContourOperation: subPage = pages["Contour"]; break;
    case DrillOperation:   subPage = pages["Drill"];   break;
    case SweepOperation:   subPage = pages["Sweep"];   break;
    }
  subPage->closeEvent(e);
  pf->beginGroup("Work");
  pf->beginWriteArray("Operations");
  for (int i=0; i < olm->rowCount(); ++i) {
      Operation* op = olm->operation(i);

      pf->setArrayIndex(i);
      op->store(pf->settings());
      }
  pf->endArray();
  pf->endGroup();
  pf->sync();
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
                  olm->removeRow(ui->lstOperations->currentIndex().row());

                  return true;
                  }
               }
            break;
       }
     }
  return false;
  }


void OperationsPage::genGCode() {
  QString     fileName;
  QFileDialog dialog(this
                   , tr("QFileDialog::getSaveFileName()")
                   , "/media/Scratch"
                   , tr("GCode Files (*.ngc)"));

  dialog.setSupportedSchemes(QStringList(QStringLiteral("file")));
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted) {
     qDebug() << "Oups - file-dialog aborted!";
     return;
     }
  fileName = dialog.selectedUrls().value(0).toLocalFile();

  qDebug() << "save gcode to:" << fileName;
  GCodeWriter gcw(new PPFanuc());
  Bnd_Box     wpBounds = Core().workData()->workPiece->BoundingBox();

  gcw.processOperations(fileName, wpBounds, olm->operations());
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
  Core().view3D()->rotate(deg2rad(dA), deg2rad(dB), deg2rad(dC));

  switch (op->kind()) {
    case ContourOperation: subPage = pages["Contour"]; break;
    case DrillOperation:   subPage = pages["Drill"];   break;
    case SweepOperation:   subPage = pages["Sweep"];   break;
    }
  opStack->setCurrentWidget(subPage);

  currentOperation = op;
  subPage->loadOP(currentOperation);

  ui->dsCut->setValue(op->waterlineDepth());
  if (currentOperation) {
     if (currentOperation->cShapes.size())     Core().view3D()->showShapes(currentOperation->cShapes);
     if (currentOperation->workSteps().size()) subPage->showToolPath();
     Core().view3D()->refresh();
     }
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
  subPage->processSelection();
  }


void OperationsPage::rotate() {
  double dA = ui->spA->value();
  double dB = ui->spB->value();
  double dC = ui->spC->value();

  if (currentOperation) Core().view3D()->removeShapes(currentOperation->toolPaths);
  Core().view3D()->rotate(deg2rad(dA), deg2rad(dB), deg2rad(dC));
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


void OperationsPage::toolPath() {
  if (!subPage) return;
  if (currentOperation->toolPaths.size()) {
     Core().view3D()->removeShapes(currentOperation->toolPaths);
     currentOperation->toolPaths.clear();
     }
  if (currentOperation->cShapes.size()) {
     Core().view3D()->removeShapes(currentOperation->cShapes);
     currentOperation->cShapes.clear();
     }
  if (currentOperation->workSteps().size()) currentOperation->workSteps().clear();
  subPage->fixit();
  subPage->toolPath();
  }
