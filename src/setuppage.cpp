/* 
 * **************************************************************************
 * 
 *  file:       setuppage.cpp
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
#include "setuppage.h"
#include "projectfile.h"
#include "ui_setup.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "core.h"
#include "util3d.h"
#include "occtviewer.h"
#include "viselistmodel.h"
#include "work.h"
#include "kuteCAM.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepTools.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <gp_Quaternion.hxx>
#include <QDoubleSpinBox>
#include <QStringListModel>
#include <QVector3D>
#include <QDebug>


SetupPage::SetupPage(StringListModel* matModel, ViseListModel* vises, QWidget *parent)
 : ApplicationWindow(parent)
 , ui(new Ui::SetupPage)
 , matModel(matModel)
 , vises(vises) {
  ui->setupUi(this);
  ui->cbVise->setModel(vises);
  enableModel(true);
  enableWorkpiece(false);
  enableClamping(false);
  ui->gbVise->setEnabled(false);
  connect(this, &SetupPage::raiseMessage, Core().mainWin(), &MainWindow::setStatusMessage);
  connect(this, &SetupPage::modelChanged, Core().mainWin(), &MainWindow::refresh);
  ui->cbMaterial->setModel(matModel);
  }


void SetupPage::changeVise(const QString &vise) {
  qDebug() << "vise changed to:" << vise;
  OcctQtViewer* view3D = Core().view3D();
  Work*         work   = Core().workData();

  view3D->removeShape(work->vl);
  if (!work->vm.IsNull()) view3D->removeShape(work->vm);
  if (!work->vr.IsNull()) view3D->removeShape(work->vr);
  int vi = ui->cbVise->currentIndex();

  Core().loadVise(vises->vise(vi), work->vl, work->vm, work->vr);
  view3D->showShape(work->vl, false);
  if (!work->vm.IsNull()) view3D->showShape(work->vm, false);
  if (!work->vr.IsNull()) view3D->showShape(work->vr, false);
  }


void SetupPage::createClamping() {
  ui->xNegClamp->setValue(10);
  ui->xPosClamp->setValue(-10);
  ui->yNegClamp->setValue(10);
  ui->yPosClamp->setValue(-10);
  ui->zNegClamp->setValue(-10);
  enableClamping(true);
  updateClampingPlug();
  }


void SetupPage::createVise() {
  Bnd_Box cBounds = Core().workData()->clampingPlug->BoundingBox();

  if (ui->cOnTop->isChecked()) cBounds = Core().workData()->workPiece->BoundingBox();
  ui->xVL->setValue(cBounds.CornerMin().X());
  ui->xVM->setValue(cBounds.CornerMin().X() + (cBounds.CornerMax().X() - cBounds.CornerMin().X()) / 2);
  ui->xVR->setValue(cBounds.CornerMax().X());

  ui->yVL->setValue(cBounds.CornerMin().Y() + (cBounds.CornerMax().Y() - cBounds.CornerMin().Y()) / 2);
  ui->yVM->setValue(cBounds.CornerMin().Y() + (cBounds.CornerMax().Y() - cBounds.CornerMin().Y()) / 2);
  ui->yVR->setValue(cBounds.CornerMin().Y() + (cBounds.CornerMax().Y() - cBounds.CornerMin().Y()) / 2);

  ui->zVL->setValue(cBounds.CornerMin().Z());
  ui->zVM->setValue(cBounds.CornerMin().Z());
  ui->zVR->setValue(cBounds.CornerMin().Z());
  changeVise(tr("Standard"));
  enableVise(true);
  updateVise();
  }


void SetupPage::createWorkPiece(Handle(AIS_Shape) model) {
  Bnd_Box wpBounds = model->BoundingBox();

  wpBounds.Enlarge(5);
  gp_Pnt cMin = wpBounds.CornerMin();
  gp_Pnt cMax = wpBounds.CornerMax();

  Core().view3D()->setBounds(wpBounds);
  cMin = gp_Pnt(floor(cMin.X()), floor(cMin.Y()), floor(cMin.Z()));
  cMax = gp_Pnt(ceil(cMax.X()),  ceil(cMax.Y()),   ceil(cMax.Z()));

  qDebug() << "min corner:" << cMin.X() << "/" << cMin.Y() << "/" << cMin.Z();
  qDebug() << "max corner:" << cMax.X() << "/" << cMax.Y() << "/" << cMax.Z();

  double xWP = cMax.X() - cMin.X();
  double yWP = cMax.Y() - cMin.Y();
  double zWP = cMax.Z() - cMin.Z();

  ui->xWSize->setValue(xWP);
  ui->yWSize->setValue(yWP);
  ui->zWSize->setValue(zWP);
  ui->wDiam->setValue(fmax(cMax.X(), cMax.Y()) - fmin(cMin.X(), cMin.Y()));
  ui->wCylHeight->setValue(cMax.Z() - cMin.Z());

  ui->xWOff->setValue(cMin.X());
  ui->yWOff->setValue(cMin.Y());
  ui->zWOff->setValue(cMin.Z());

  qDebug() << "createWorkPiece: wp-offset" << ui->xWOff->value() << "/" << ui->yWOff->value() << "/" << ui->zWOff->value();

  if (!ui->xWSize->value()
   || !ui->yWSize->value()
   || !ui->zWSize->value()) return;

  updateWorkPiece();
  enableWorkpiece(true);
  }


void SetupPage::cylToggle(int state) {
  Core().view3D()->removeShape(Core().workData()->workPiece);

  ui->lDiameter->setEnabled(state);
  ui->lZDia->setEnabled(state);
  ui->wDiam->setEnabled(state);
  ui->wCylHeight->setEnabled(state);

  ui->lXSize->setEnabled(!state);
  ui->lYSize->setEnabled(!state);
  ui->lZSize->setEnabled(!state);
  ui->xWSize->setEnabled(!state);
  ui->yWSize->setEnabled(!state);
  ui->zWSize->setEnabled(!state);

  if (state) {
     ui->xWOff->setValue(0);
     ui->yWOff->setValue(0);
     ui->zWOff->setValue(-5);
     ui->wDiam->setFocus();
     }
  else {
     ui->xWOff->setValue(-0.5 * ui->xWSize->value());
     ui->yWOff->setValue(-0.5 * ui->yWSize->value());
     ui->zWOff->setValue(-0.5 * ui->zWSize->value());
     ui->xWSize->setFocus();
     }
  updateWorkPiece();
  }


void SetupPage::enableClamping(bool enabled) {
  emit raiseMessage(tr("resize clamping plug, then press >Set<"));
  ui->gbClamping->setEnabled(enabled);
  if (enabled) {
     connect(ui->xNegClamp, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateClampingPlug);
     connect(ui->xPosClamp, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateClampingPlug);
     connect(ui->yNegClamp, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateClampingPlug);
     connect(ui->yPosClamp, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateClampingPlug);
     connect(ui->zNegClamp, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateClampingPlug);
     connect(ui->pbCSet,    &QPushButton::clicked,         this, &SetupPage::fixClamping);
     connect(ui->cOnTop,    &QCheckBox::toggled,           this, &SetupPage::onTopToggle);
     }
  else {
     disconnect(ui->xNegClamp, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateClampingPlug);
     disconnect(ui->xPosClamp, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateClampingPlug);
     disconnect(ui->yNegClamp, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateClampingPlug);
     disconnect(ui->yPosClamp, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateClampingPlug);
     disconnect(ui->zNegClamp, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateClampingPlug);
     disconnect(ui->pbCSet,    &QPushButton::clicked,         this, &SetupPage::fixClamping);
     disconnect(ui->cOnTop,    &QCheckBox::toggled,           this, &SetupPage::onTopToggle);
     }
  }


void SetupPage::enableModel(bool enabled) {
  emit raiseMessage(tr("move and rotate model, then press >Set<"));
  ui->gbModel->setEnabled(enabled);
  if (enabled) {
     connect(ui->angABase,   &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     connect(ui->angBBase,   &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     connect(ui->angCBase,   &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     connect(ui->xMOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     connect(ui->yMOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     connect(ui->zMOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     connect(ui->pbMSet,     &QPushButton::clicked,         this, &SetupPage::fixModel);
     }
  else {
     disconnect(ui->angABase,   &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     disconnect(ui->angBBase,   &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     disconnect(ui->angCBase,   &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     disconnect(ui->xMOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     disconnect(ui->yMOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     disconnect(ui->zMOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::transformModel);
     disconnect(ui->pbMSet,     &QPushButton::clicked,         this, &SetupPage::fixModel);
     }
  }


void SetupPage::enableWorkpiece(bool enabled) {
  emit raiseMessage(tr("move and resize workpiece, then press >Set<"));
  ui->gbWorkpiece->setEnabled(enabled);
  if (enabled) {
     connect(ui->xWSize,     &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     connect(ui->yWSize,     &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     connect(ui->zWSize,     &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     connect(ui->xWOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     connect(ui->yWOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     connect(ui->zWOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     connect(ui->wDiam,      &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     connect(ui->wCylHeight, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     connect(ui->cbCylinder, &QCheckBox::toggled,           this, &SetupPage::cylToggle);
     connect(ui->pbWSet,     &QPushButton::clicked,         this, &SetupPage::fixWorkpiece);
     }
  else {
     disconnect(ui->xWSize,     &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     disconnect(ui->yWSize,     &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     disconnect(ui->zWSize,     &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     disconnect(ui->xWOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     disconnect(ui->yWOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     disconnect(ui->zWOff,      &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     disconnect(ui->wDiam,      &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     disconnect(ui->wCylHeight, &QDoubleSpinBox::valueChanged, this, &SetupPage::updateWorkPiece);
     disconnect(ui->cbCylinder, &QCheckBox::toggled,           this, &SetupPage::cylToggle);
     disconnect(ui->pbWSet,     &QPushButton::clicked,         this, &SetupPage::fixWorkpiece);
     }
  }


void SetupPage::enableVise(bool enabled) {
  emit raiseMessage(tr("move vise, then press >Set<"));
  ui->gbVise->setEnabled(enabled);
  if (enabled) {
     connect(ui->xVL,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     connect(ui->xVM,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     connect(ui->xVR,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     connect(ui->yVL,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     connect(ui->yVM,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     connect(ui->yVR,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     connect(ui->zVL,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     connect(ui->zVM,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     connect(ui->zVR,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     connect(ui->pbVise, &QPushButton::clicked,          this, &SetupPage::fixVise);
     connect(ui->cbVise, &QComboBox::currentTextChanged, this, &SetupPage::changeVise);
     }
  else {
     disconnect(ui->xVL,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     disconnect(ui->xVM,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     disconnect(ui->xVR,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     disconnect(ui->yVL,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     disconnect(ui->yVM,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     disconnect(ui->yVR,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     disconnect(ui->zVL,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     disconnect(ui->zVM,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     disconnect(ui->zVR,    &QDoubleSpinBox::valueChanged,  this, &SetupPage::updateVise);
     disconnect(ui->pbVise, &QPushButton::clicked,          this, &SetupPage::fixVise);
     disconnect(ui->cbVise, &QComboBox::currentTextChanged, this, &SetupPage::changeVise);
     }
  }


void SetupPage::exploreModel(const TopoDS_Shape& shape) {
  Core().workData()->modShapes = Core().helper3D()->explodeShape(shape);
  Bnd_Box bb = Core().workData()->workPiece->BoundingBox();

  for (auto& s : Core().workData()->modShapes) {
      s->SetTransparency(0.5);
      Core().view3D()->showShape(s);      
      }
  std::vector<TopoDS_Face> wpFaces = Core().helper3D()->allFacesWithin(Core().workData()->workPiece->Shape());

  for (auto f : wpFaces) {
      std::vector<gp_Pnt> corners = Core().helper3D()->allVertexCoordinatesWithin(f);
      int found = 0;

      qDebug() << "exploreModel() - print corners of workpiece face:";

      for (auto c : corners) {
          qDebug() << "    c:" << c.X() << " / " << c.Y() << " / " << c.Z();

          if (kute::isEqual(c.Z(), bb.CornerMin().Z()))
             ++found;
          }
      if (found == corners.size()) {
         Core().view3D()->setBaseFace(new AIS_Shape(f));
         break;
         }
      }
  Core().view3D()->showShape(Core().view3D()->baseFace());
  Core().view3D()->fitAll();
  }


void SetupPage::fixClamping() {
  Work*               work = Core().workData();
  gp_Trsf             t    = work->clampingPlug->Transformation();
  const TopoDS_Shape& base = work->clampingPlug->Shape();
  TopoDS_Shape        nCP  = base.Located(TopLoc_Location(t));

  qDebug() << "model fixed?!?";

  Core().projectFile()->beginGroup("Setup");
  Core().projectFile()->setValue("clamping-onTop", ui->cOnTop->isChecked());
  Core().projectFile()->setValue("clamping-min", QVector3D(ui->xNegClamp->value()
                                       , ui->yNegClamp->value()
                                       , ui->zNegClamp->value()));
  Core().projectFile()->setValue("clamping-max", QVector3D(ui->xPosClamp->value()
                                       , ui->yPosClamp->value()
                                       , ui->zPosClamp->value()));
  Core().projectFile()->endGroup();
  Core().view3D()->removeShape(Core().workData()->clampingPlug);
  work->clampingPlug = new AIS_Shape(nCP);
  work->clampingPlug->SetColor(Quantity_NOC_BLUE);
  work->clampingPlug->SetTransparency(0.7);
  work->cpOnTop = ui->cOnTop->isChecked();
  Core().view3D()->showShape(work->clampingPlug, false);
  Core().view3D()->removeShape(work->model);
  enableClamping(false);
  createVise();
  }


void SetupPage::fixModel() {
  gp_Trsf             t      = Core().workData()->model->Transformation();
  const TopoDS_Shape& base   = Core().workData()->model->Shape();
  TopoDS_Shape        nModel = base.Located(TopLoc_Location(t));
  std::stringstream   ss;

  t.DumpJson(ss);
  std::string st = ss.str();
  ProjectFile* pf = Core().projectFile();

  pf->beginGroup("Setup");
  pf->setValue("model-trans", st.c_str());
  pf->setValue("model-loc", QVector3D(ui->xMOff->value()
                                    , ui->yMOff->value()
                                    , ui->zMOff->value()));
  pf->setValue("model-rot", QVector3D(ui->angABase->value()
                                    , ui->angBBase->value()
                                    , ui->angCBase->value()));
  pf->setValue("model-comment", ui->comment->document()->toPlainText());
  pf->endGroup();
  qDebug() << "model loc:" << ui->xMOff->value()
           << "/"          << ui->yMOff->value()
           << "/"          << ui->zMOff->value();
  qDebug() << "model rot:" << ui->angABase->value()
           << " + "        << ui->angBBase->value()
           << " + "        << ui->angCBase->value();
  Work* work = Core().workData();

  Core().view3D()->removeShape(work->model);
  work->model = new AIS_Shape(nModel);
  Core().view3D()->showShape(work->model);

  ui->xMOff->setValue(0);
  ui->yMOff->setValue(0);
  ui->zMOff->setValue(0);
  ui->angABase->setValue(0);
  ui->angBBase->setValue(0);
  ui->angCBase->setValue(0);

  enableModel(false);
  createWorkPiece(work->model);
  enableWorkpiece(true);
  }


void SetupPage::fixVise() {
  Work*   work = Core().workData();
  Bnd_Box cBounds = work->clampingPlug->BoundingBox();
  gp_Trsf tVL = work->vl->Transformation();
  gp_Trsf mVL, mVM, mVR;

  if (ui->cOnTop->isChecked()) cBounds = work->workPiece->BoundingBox();
  mVL.SetTranslation({0, 0, 0}, {cBounds.CornerMin().X()
                               , ui->yVL->value()
                               , ui->zVL->value()});
  const TopoDS_Shape& bVL    = work->vl->Shape();
  TopoDS_Shape        nVL    = bVL.Moved(TopLoc_Location(mVL));
  ProjectFile*        pf     = Core().projectFile();
  OcctQtViewer*       view3D = Core().view3D();

  pf->beginGroup("Setup");
  pf->setValue("vise-type", ui->cbVise->currentText());
  pf->setValue("vise-left", QVector3D(cBounds.CornerMin().X()
                                    , ui->yVL->value()
                                    , ui->zVL->value()));

  view3D->removeShape(work->vl);
  work->vl = new AIS_Shape(nVL);
  if (!work->vm.IsNull()) {
     gp_Trsf tVM = work->vm->Transformation();

     mVM.SetTranslation({0, 0, 0}, {cBounds.CornerMax().X() -
                                   (cBounds.CornerMax().X() - cBounds.CornerMin().X()) / 2
                                  , ui->yVM->value()
                                  , ui->zVM->value()});
     pf->setValue("vise-middle", QVector3D(cBounds.CornerMax().X() -
                                          (cBounds.CornerMax().X() - cBounds.CornerMin().X()) / 2
                                         , ui->yVR->value()
                                         , ui->zVR->value()));
     const TopoDS_Shape& bVM = work->vm->Shape();
     TopoDS_Shape        nVM = bVM.Moved(TopLoc_Location(mVM));
     view3D->removeShape(work->vm);
     work->vm = new AIS_Shape(nVM);
     work->vm->SetColor(Quantity_NOC_GRAY);
     view3D->showShape(work->vm, false);
     }
  if (!work->vr.IsNull()) {
     gp_Trsf tVR = work->vr->Transformation();

     mVR.SetTranslation({0, 0, 0}, {cBounds.CornerMax().X()
                                  , ui->yVR->value()
                                  , ui->zVR->value()});
     const TopoDS_Shape& bVR = work->vr->Shape();
     TopoDS_Shape        nVR = bVR.Moved(TopLoc_Location(mVR));

     pf->setValue("vise-right", QVector3D(cBounds.CornerMax().X()
                                        , ui->yVR->value()
                                        , ui->zVR->value()));
     view3D->removeShape(work->vr);
     work->vr = new AIS_Shape(nVR);
     work->vr->SetColor(Quantity_NOC_GRAY);
     view3D->showShape(work->vr, false);
     }
  pf->endGroup();
  Bnd_Box bb = work->vl->BoundingBox();

  if (!work->vm.IsNull()) bb.Add(work->vm->BoundingBox());
  if (!work->vr.IsNull()) bb.Add(work->vr->BoundingBox());
  work->vise = new AIS_Shape(BRepPrimAPI_MakeBox(bb.CornerMin(), bb.CornerMax()).Shape());
  work->vise->SetColor(Quantity_NOC_GRAY);
  work->vise->SetTransparency(0.6);
  work->vl->SetColor(Quantity_NOC_GRAY);
  view3D->showShape(work->vl, false);
  enableVise(false);
  exploreModel(work->model->Shape());
  setupDone();
  }


void SetupPage::fixWorkpiece() {
  OcctQtViewer* view3D = Core().view3D();
  Work*         work   = Core().workData();
  ProjectFile*  pf     = Core().projectFile();

  view3D->removeShape(work->workPiece);
  work->workPiece = Core().helper3D()->fixLocation(work->workPiece->Shape()
                                                 , ui->xWOff->value()
                                                 , ui->yWOff->value()
                                                 , ui->zWOff->value());
  work->workPiece->SetColor(Quantity_NOC_BLUE);
  work->workPiece->SetTransparency(0.8);

  qDebug() << "workpiece fixed?!?";

  pf->beginGroup("Setup");
  pf->setValue("workpiece-is-cylinder", ui->cbCylinder->isChecked());
  pf->setValue("workpiece-pos", QVector3D(ui->xWOff->value()
                                        , ui->yWOff->value()
                                        , ui->zWOff->value()));
  if (ui->cbCylinder->isChecked()) {
     pf->setValue("workpiece-size", QVector3D(ui->wDiam->value() / 2
                                            , ui->wCylHeight->value()
                                            , 0));
     }
  else {
     pf->setValue("workpiece-size", QVector3D(ui->xWSize->value()
                                            , ui->yWSize->value()
                                            , ui->zWSize->value()));
     }
  pf->setValue("workpiece-material", ui->cbMaterial->currentText());
  work->material = ui->cbMaterial->currentText();
  pf->endGroup();
  view3D->setWorkpiece(work->workPiece);
  view3D->showShape(work->workPiece, false);
  view3D->setBounds(work->workPiece->BoundingBox());
  enableWorkpiece(false);
  ui->xWOff->setValue(0);
  ui->yWOff->setValue(0);
  ui->zWOff->setValue(0);
  createClamping();
  }


void SetupPage::loadProject(ProjectFile* pf, const TopoDS_Shape& mShape) {
  if (!pf) return;
  OcctQtViewer* view3D = Core().view3D();
  Work*         work   = Core().workData();
  Util3D*       helper = Core().helper3D();

  view3D->reset3D();
  pf->beginGroup("Setup");
  QVector3D     qloc = pf->value("model-loc").value<QVector3D>();
  QVector3D     qrot = pf->value("model-rot").value<QVector3D>();
  gp_Quaternion q;
  gp_Trsf       t;

  q.SetEulerAngles(gp_Intrinsic_XYZ
                 , kute::deg2rad(qrot.x())
                 , kute::deg2rad(qrot.y())
                 , kute::deg2rad(qrot.z()));
  t.SetTransformation(q, {qloc.x(), qloc.y(), qloc.z()});
  BRepBuilderAPI_Transform trans(t);

  trans.Perform(mShape, true);
  work->model = new AIS_Shape(trans.Shape());
  emit modelChanged(work->model->BoundingBox());
  TopoDS_Shape tmp;

  qloc = pf->value("workpiece-size").value<QVector3D>();
  if (pf->value("workpiece-is-cylinder").toBool()) {
     work->roundWorkPiece = true;
     tmp = BRepPrimAPI_MakeCylinder(qloc.x()
                                  , qloc.y()).Shape();
     }
 else {
     work->roundWorkPiece = false;
     tmp = BRepPrimAPI_MakeBox(qloc.x()
                             , qloc.y()
                             , qloc.z()).Shape();
     }
  qloc = pf->value("workpiece-pos").value<QVector3D>();
  work->workPiece = helper->fixLocation(tmp
                                      , qloc.x()
                                      , qloc.y()
                                      , qloc.z());
  work->workPiece->SetColor(Quantity_NOC_BLUE);
  work->workPiece->SetTransparency(0.8);
  view3D->setWorkpiece(work->workPiece);
  Bnd_Box bb = work->workPiece->BoundingBox();

  view3D->showShape(work->workPiece, false);
  exploreModel(work->model->Shape());
  ui->cbMaterial->setCurrentText(pf->value("workpiece-material").toString());
  work->material = ui->cbMaterial->currentText();
  view3D->setBounds(bb);
  qloc = pf->value("clamping-min").value<QVector3D>();
  qrot = pf->value("clamping-max").value<QVector3D>();
  gp_Pnt p0 = bb.CornerMin();
  gp_Pnt p1 = bb.CornerMax();

  p0.SetX(p0.X() + qloc.x());
  p0.SetY(p0.Y() + qloc.y());
  p1.SetX(p1.X() + qrot.x());
  p1.SetY(p1.Y() + qrot.y());
  if (pf->value("clamping-onTop").toBool()) {
     work->cpOnTop = true;
     p0.SetZ(p1.Z());
     p1.SetZ(p1.Z() + qrot.z());
     }
  else {
     work->cpOnTop = false;
     p0.SetZ(p0.Z());
     p1.SetZ(p0.Z() + qloc.z());
     }
  tmp = BRepPrimAPI_MakeBox(p0, p1).Shape();
  work->clampingPlug = new AIS_Shape(tmp);
  work->clampingPlug->SetColor(Quantity_NOC_BLUE);
  work->clampingPlug->SetTransparency(0.7);
  view3D->showShape(work->clampingPlug, false);
  QString    viseName = pf->value("vise-type").toString();
  ViseEntry* ve       = vises->find(viseName);
  assert(ve != nullptr);
  Core().loadVise(ve, work->vl, work->vm, work->vr);
  qloc = pf->value("vise-left").value<QVector3D>();
  gp_Trsf mVL, mVM, mVR;

  mVL.SetTranslation({0, 0, 0}, {qloc.x()
                               , qloc.y()
                               , qloc.z()});
  tmp = work->vl->Shape().Moved(TopLoc_Location(mVL));
  work->vl  = new AIS_Shape(tmp);

  if (!work->vm.IsNull()) {
     qloc = pf->value("vise-middle").value<QVector3D>();
     mVM.SetTranslation({0, 0, 0}, {qloc.x()
                                  , qloc.y()
                                  , qloc.z()});
     tmp = work->vm->Shape().Moved(TopLoc_Location(mVM));
     work->vm  = new AIS_Shape(tmp);
     work->vm->SetColor(Quantity_NOC_GRAY);
     view3D->showShape(work->vm, false);
     }
  if (!work->vr.IsNull()) {
     qrot = pf->value("vise-right").value<QVector3D>();
     mVR.SetTranslation({0, 0, 0}, {qrot.x()
                                  , qrot.y()
                                  , qrot.z()});
     tmp = work->vr->Shape().Moved(TopLoc_Location(mVR));
     work->vr  = new AIS_Shape(tmp);
     work->vr->SetColor(Quantity_NOC_GRAY);
     view3D->showShape(work->vr, false);
     }
  work->vl->SetColor(Quantity_NOC_GRAY);
  view3D->showShape(work->vl, false);
  bb = work->vl->BoundingBox();
  if (!work->vm.IsNull()) bb.Add(work->vm->BoundingBox());
  if (!work->vr.IsNull()) bb.Add(work->vr->BoundingBox());
  work->vise = new AIS_Shape(BRepPrimAPI_MakeBox(bb.CornerMin(), bb.CornerMax()).Shape());
  enableModel(false);
  view3D->iso1View();
  pf->endGroup();
  }


void SetupPage::onTopToggle() {
  if (ui->cOnTop->isChecked()) {
     ui->zNegClamp->setEnabled(false);
     ui->zPosClamp->setEnabled(true);
     ui->zNegClamp->setValue(0);
     ui->zPosClamp->setValue(10);
     }
  else {
     ui->zNegClamp->setEnabled(true);
     ui->zPosClamp->setEnabled(false);
     ui->zPosClamp->setValue(0);
     ui->zNegClamp->setValue(-10);
     }
  updateClampingPlug();
  }


void SetupPage::transformModel() {
  double        dA = ui->angABase->value();
  double        dB = ui->angBBase->value();
  double        dC = ui->angCBase->value();
  double        x  = ui->xMOff->value();
  double        y  = ui->yMOff->value();
  double        z  = ui->zMOff->value();
  gp_Quaternion q;
  gp_Trsf       trans;

  qDebug() << "SetupPage - transform model with rotation:" << dA << " / " << dB << " / " << dC
                                          << "\tand move:" << x  << " / " << y  << " / " << z;

  q.SetEulerAngles(gp_Intrinsic_XYZ
                 , kute::deg2rad(dA)
                 , kute::deg2rad(dB)
                 , kute::deg2rad(dC));
  trans.SetTransformation(q, {x, y, z});

  Core().workData()->model->SetLocalTransformation(trans);
  emit modelChanged(Core().workData()->model->BoundingBox());
  Core().view3D()->refresh();
  }


void SetupPage::setModel(const TopoDS_Shape& shape) {
  OcctQtViewer* view3D = Core().view3D();
  Work*         work   = Core().workData();

  view3D->reset3D();
  work->model = new AIS_Shape(shape);
  view3D->setMainShape(work->model);
  view3D->showShape(work->model);
  emit modelChanged(work->model->BoundingBox());
  view3D->refresh();
  enableModel(true);
  }


void SetupPage::setupDone() {
  Core().uiMainWin()->actionModelSetup->setEnabled(false);
  Core().uiMainWin()->actionOperationsSetup->setEnabled(true);
  emit raiseMessage(tr("select element from model and create Operation"));
  Core().switchPage(Core::PgOperations);
  }


void SetupPage::updateClampingPlug() {
  Bnd_Box xt  = Core().workData()->workPiece->BoundingBox();
  double xNeg = ui->xNegClamp->value();
  double xPos = ui->xPosClamp->value();
  double yNeg = ui->yNegClamp->value();
  double yPos = ui->yPosClamp->value();
  double zNeg = ui->zNegClamp->value();
  double zPos = ui->zPosClamp->value();
  OcctQtViewer* view3D = Core().view3D();
  gp_Pnt p0 = xt.CornerMin();
  gp_Pnt p1 = xt.CornerMax();

  if (ui->cOnTop->isChecked()) {
     p0.SetX(p0.X() + xNeg);
     p0.SetY(p0.Y() + yNeg);
     p0.SetZ(p1.Z());
     p1.SetX(p1.X() + xPos);
     p1.SetY(p1.Y() + yPos);
     p1.SetZ(p1.Z() + zPos);
     }
  else {
     p0.SetX(p0.X() + xNeg);
     p0.SetY(p0.Y() + yNeg);
     p0.SetZ(p0.Z());
     p1.SetX(p1.X() + xPos);
     p1.SetY(p1.Y() + yPos);
     p1.SetZ(p0.Z() + zNeg);
     }

  TopoDS_Shape box = BRepPrimAPI_MakeBox(p0, p1).Shape();

  if (!Core().workData()->clampingPlug.IsNull()) view3D->removeShape(Core().workData()->clampingPlug);
  Core().workData()->clampingPlug = new AIS_Shape(box);
  view3D->showShape(Core().workData()->clampingPlug, false);
  view3D->refresh();
  }


void SetupPage::updateVise() {
  gp_Trsf       mL, mM, mR;
  OcctQtViewer* view3D = Core().view3D();
  Work*         work   = Core().workData();

  mL.SetTranslation({0, 0, 0}, {ui->xVL->value()
                              , ui->yVL->value()
                              , ui->zVL->value()});
  work->vl->SetLocalTransformation(mL);
  work->vl->SetColor(Quantity_NOC_GRAY);
  view3D->showShape(work->vl, false);

  if (!work->vm.IsNull()) {
     mM.SetTranslation({0, 0, 0}, {ui->xVM->value()
                                 , ui->yVM->value()
                                 , ui->zVM->value()});
     work->vm->SetLocalTransformation(mM);
     work->vm->SetColor(Quantity_NOC_GRAY);
     view3D->showShape(work->vm, false);
     }
  if (!work->vr.IsNull()) {
     mR.SetTranslation({0, 0, 0}, {ui->xVR->value()
                                 , ui->yVR->value()
                                 , ui->zVR->value()});
     work->vr->SetLocalTransformation(mR);
     work->vr->SetColor(Quantity_NOC_GRAY);
     view3D->showShape(work->vr, false);
     }
  view3D->refresh();
  }


void SetupPage::updateWorkPiece() {
  OcctQtViewer* view3D = Core().view3D();
  Work*         work   = Core().workData();
  if (!work->workPiece.IsNull())  view3D->removeShape(work->workPiece);
  if (ui->cbCylinder->isChecked()) {
     TopoDS_Shape cylinder = BRepPrimAPI_MakeCylinder(ui->wDiam->value() / 2
                                                    , ui->wCylHeight->value()).Shape();

     work->workPiece = new AIS_Shape(cylinder);
     }
  else {
     TopoDS_Shape box = BRepPrimAPI_MakeBox(ui->xWSize->value()
                                          , ui->yWSize->value()
                                          , ui->zWSize->value()).Shape();

     work->workPiece = new AIS_Shape(box);
     }
  gp_Trsf       move;
  gp_Quaternion q;

  move.SetTranslation({0,0,0}, {ui->xWOff->value(), ui->yWOff->value(), ui->zWOff->value()});
  work->workPiece->SetLocalTransformation(move);
  work->workPiece->SetColor(Quantity_NOC_BLUE);
  work->workPiece->SetTransparency(0.7);
  view3D->showShape(work->workPiece, false);
  view3D->setWorkpiece(work->workPiece);
  view3D->refresh();
  }
