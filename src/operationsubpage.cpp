/* 
 * **************************************************************************
 * 
 *  file:       operationsubpage.cpp
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
#include "operationsubpage.h"
#include "mainwindow.h"
#include "ui_opSub.h"
#include "core.h"
#include "cuttingparameters.h"
#include "occtviewer.h"
#include "pathbuilder.h"
#include "targetdeflistmodel.h"
#include "toolentry.h"
#include "toollistmodel.h"
#include "util3d.h"
#include "work.h"
#include <BRepAdaptor_Surface.hxx>
#include <QStringListModel>
#include <QDebug>


OperationSubPage::OperationSubPage(OperationListModel* olm, TargetDefListModel* tdModel, QWidget *parent)
 : QWidget(parent)
 , ui(new Ui::OpSub)
 , olm(olm)
 , curOP(nullptr)
 , activeTool(nullptr)
 , pathBuilder(new PathBuilder)
 , tdModel(tdModel)
 , opTypes(nullptr) {
  ui->setupUi(this);
  QStringList items;

  items << Operation::OTRoughing
        << Operation::OTFinish;
  opTypes = new QStringListModel(items, this);

  items.clear();
  items << tr("parallel feed")
        << tr("against feed")
        << tr("don't care");
  dirModel = new QStringListModel(items, this);

  items.clear();
  items = genCycleList();
  drillCycles = new QStringListModel(items, this);

  items.clear();
  items << tr("No cooling")
        << tr("Mist cooling")
        << tr("Flood cooling");
  coolingModes = new QStringListModel(items, this);

  items.clear();
  items << "G53"
        << "G54"
        << "G55"
        << "G56"
        << "G57"
        << "G58"
        << "G59"
        << "G59.1"
        << "G59.2"
        << "G59.3";
  fixModel = new QStringListModel(items, this);
  ui->cbTool->setModel(Core().toolListModel());
  ui->cbCooling->setModel(coolingModes);
  ui->cbFixture->setModel(fixModel);
  ui->cbCycle->setModel(drillCycles);
  ui->cbDir->setModel(dirModel);
  ui->cbType->setModel(opTypes);
  }


void OperationSubPage::absToggled(const QVariant& v) {
  curOP->setAbsolute(v.toBool());
  }


void OperationSubPage::closeEvent(QCloseEvent *e) {
  }


void OperationSubPage::connectSignals() {
  connect(ui->cAbsolute, &QCheckBox::toggled, this, &OperationSubPage::absToggled);
  connect(ui->cOutside,  &QCheckBox::toggled, this, &OperationSubPage::outToggled);
  connect(ui->cbTool,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OperationSubPage::toolChanged);
  connect(ui->cbCooling, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OperationSubPage::coolingChanged);
  connect(ui->cbCycle,   QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OperationSubPage::cycleChanged);
  connect(ui->cbDir,     QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OperationSubPage::dirChanged);
  connect(ui->cbFixture, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OperationSubPage::fixtureChanged);
  connect(ui->cbType,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OperationSubPage::typeChanged);
  connect(ui->spAe,     &QDoubleSpinBox::valueChanged, this, &OperationSubPage::cutWidthChanged);
  connect(ui->spAp,     &QDoubleSpinBox::valueChanged, this, &OperationSubPage::cutDepthChanged);
  connect(ui->spFz,     &QDoubleSpinBox::valueChanged, this, &OperationSubPage::cutFeedChanged);
  connect(ui->spVc,     &QDoubleSpinBox::valueChanged, this, &OperationSubPage::cutSpeedChanged);
  connect(ui->spDepth,  &QDoubleSpinBox::valueChanged, this, &OperationSubPage::finalDepthChanged);
  connect(ui->spOff,    &QDoubleSpinBox::valueChanged, this, &OperationSubPage::offsetChanged);
  connect(ui->spR1,     &QDoubleSpinBox::valueChanged, this, &OperationSubPage::r1Changed);
  connect(ui->spR2,     &QDoubleSpinBox::valueChanged, this, &OperationSubPage::r2Changed);
  connect(ui->opName,   &QLineEdit::textChanged,       this, &OperationSubPage::opNameChanged);
  connect(this, &OperationSubPage::modelChanged, Core().mainWin(), &MainWindow::refresh);
  }


void OperationSubPage::coolingChanged(const QVariant& v) {
  curOP->setCooling(v.toInt());
  }


// called from all child classes, so save possible changes from ui to old operation
// before creating a new one.
Operation* OperationSubPage::createOP(int id, const QString& name, OperationType type) {
  if (curOP) {
     if (curOP->toolPaths.size()) {
        Core().view3D()->removeShapes(curOP->toolPaths);
        curOP->toolPaths.clear();
        }
     if (curOP->cShapes.size()) {
        Core().view3D()->removeShapes(curOP->cShapes);
        curOP->cShapes.clear();
        }
     }
  curOP = new Operation(id, type);
  curOP->setName(name);
  connectSignals();

  emit opCreated(curOP);

  return curOP;
  }


void OperationSubPage::cutDepthChanged(double v) {
  curOP->setCutDepth(v);
  }


void OperationSubPage::cutFeedChanged(double v) {
  curOP->setFeedPerTooth(v);
  if (!activeTool) return;
  double ss = curOP->speed() * 1000 / M_PI / activeTool->fluteDiameter();
  double cf = ss * activeTool->numFlutes() * curOP->feedPerTooth();

  ui->lFeed->setText(QString("F %1").arg(cf, 0, 'f', 0));
  }


void OperationSubPage::cutSpeedChanged(double v) {
  curOP->setSpeed(v);
  if (!activeTool) return;
  double ss = curOP->speed() * 1000 / M_PI / activeTool->fluteDiameter();

  ui->lSpeed->setText(QString("S %1").arg(ss, 0, 'f', 0));
  cutFeedChanged(curOP->feedPerTooth());
  }


void OperationSubPage::cutWidthChanged(double v) {
  curOP->setCutWidth(v);
  }


void OperationSubPage::cycleChanged(const QVariant& v) {
  curOP->setDrillCycle(v.toInt());
  }


void OperationSubPage::dirChanged(const QVariant& v) {
  curOP->setDirection(static_cast<CutDirection>(v.toInt()));
  }


void OperationSubPage::finalDepthChanged(double v) {
  curOP->setFinalDepth(v);
  }


void OperationSubPage::fixit() {
  Handle(AIS_Shape) wp = Core().helper3D()->fixRotation(Core().workData()->workPiece->Shape()
                                                      , curOP->operationA()
                                                      , curOP->operationB()
                                                      , curOP->operationC());
  Handle(AIS_Shape) md = Core().helper3D()->fixRotation(Core().workData()->model->Shape()
                                                      , curOP->operationA()
                                                      , curOP->operationB()
                                                      , curOP->operationC());
  Handle(AIS_Shape) vs = Core().helper3D()->fixRotation(Core().workData()->vise->Shape()
                                                      , curOP->operationA()
                                                      , curOP->operationB()
                                                      , curOP->operationC());

  curOP->wpBounds  = wp->BoundingBox();
  curOP->mBounds   = md->BoundingBox();
  curOP->vBounds   = vs->BoundingBox();
  curOP->workPiece = wp;
  emit modelChanged(curOP->mBounds);
  wp.Nullify();
  vs.Nullify();
  }


void OperationSubPage::fixtureChanged(int i) {
  curOP->setFixture(i);
  }


QStringList OperationSubPage::genCycleList() {
  QStringList rv;

  for (int i=0; i < InvalidCycle; ++i) {
      switch (i) {
        case NO_Cycle:        rv << tr("G80 - no Cycle"); break;
        case FineBoringCycle: rv << tr("G76 - fine Boring Cycle"); break;
        case SpotDrillCycle:  rv << tr("G81 - Spot Drill Cycle"); break;
        case DrillWithDwell:  rv << tr("G82 - Drill with Dwell"); break;
        case PeckDrilling:    rv << tr("G83 - Peck Drilling"); break;
        case Tapping:         rv << tr("G84 - Tapping"); break;
        case BoringCycle:     rv << tr("G85 - Boring Cycle"); break;
        case InvalidCycle:
        default: break;
        }
      }
  return rv;
  }


void OperationSubPage::loadOP(Operation *op) {
  curOP = op;
  int ti = Core().toolListModel()->findToolNum(op->toolNum());

  qDebug() << "load operation, tool #" << op->toolNum() << "\t(" << ti << ")";

  if (ti >= 0) {
     qDebug() << "change tool combobox to index:" << ti;

     ui->cbTool->setCurrentIndex(ti);
     activeTool = Core().toolListModel()->tool(ti);
     }
  else ui->cbTool->clear();
  tdModel->replaceData(&op->targets);
  ui->opName->setText(op->name());
  ui->cOutside->setChecked(op->isOutside());
  ui->cbType->setCurrentIndex(op->cutType());
  ui->cbDir->setCurrentIndex(op->direction());
  ui->cbFixture->setCurrentIndex(op->fixture());
  ui->cbCooling->setCurrentIndex(op->cooling());
  ui->cbCycle->setCurrentIndex(op->drillCycle());
  ui->spAe->setValue(op->cutWidth());
  ui->spAp->setValue(op->cutDepth());
  ui->spFz->setValue(op->feedPerTooth());
  ui->spVc->setValue(op->speed());
  cutSpeedChanged(op->speed());
  cutFeedChanged(op->feedPerTooth());
  ui->spOff->setValue(op->offset());
  ui->cAbsolute->setChecked(op->isAbsolute());
  ui->spDepth->setValue(op->finalDepth());
  ui->spR1->setValue(op->safeZ0());
  ui->spR2->setValue(op->safeZ1());
  connectSignals();

  fixit();
  if (curOP->cShapes.size()) {
     Core().view3D()->removeShapes(curOP->cShapes);
     curOP->cShapes.clear();
     }
  if (!tdModel->rowCount()) processSelection();
  else                      processTargets();
  }


void OperationSubPage::offsetChanged(double v) {
  curOP->setOffset(v);
  }


void OperationSubPage::opNameChanged(const QString& name) {
  curOP->setName(name);
  }


void OperationSubPage::outToggled(const QVariant& v) {
  curOP->setOutside(v.toBool());
  }


void OperationSubPage::processTargets() {
  }


void OperationSubPage::r1Changed(double v) {
  curOP->setSaveZ0(v);
  }


void OperationSubPage::r2Changed(double v) {
  curOP->setSaveZ1(v);
  }


void OperationSubPage::toolChanged(const QVariant &i) {
  activeTool = Core().toolListModel()->tool(i.toInt());

  if (!activeTool) return;
  curOP->setToolNum(activeTool->toolNumber());
  if (Core().helper3D()->isEqual(curOP->speed(), 0)) {
     CuttingParameters* cp = activeTool->cutParameter(Core().workData()->material);

     if (!cp) return;
     ui->spVc->setValue(cp->cuttingSpeed());
     ui->spFz->setValue(cp->toothFeed());
     ui->spAe->setValue(cp->widthOfCut());
     ui->spAp->setValue(cp->depthOfCut());
     }
  }


// switch between roughing and finishing
void OperationSubPage::typeChanged(const QVariant& v) {
  CutType ct = static_cast<CutType>(v.toInt());

  curOP->setCutType(ct);
  }
