/* 
 * **************************************************************************
 * 
 *  file:       cfgvise.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    15.5.2022 by Django Reinhard
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
#include "cfgvise.h"
#include "ui_cfgVise.h"
#include "core.h"
#include "occtviewer.h"
#include "viselistmodel.h"
#include "viseentry.h"
#include <QDebug>


CfgVise::CfgVise(ViseListModel* vModel, QWidget *parent)
 : QWidget(parent)
 , ui(new Ui::ViseConfig)
 , vModel(vModel) {
  ui->setupUi(this);
  ui->lstVise->setModel(vModel);

  connect(ui->pbLeftOpen,   &QPushButton::clicked, this, &CfgVise::setLeftVise);
  connect(ui->pbMidOpen,    &QPushButton::clicked, this, &CfgVise::setMidVise);
  connect(ui->pbRightOpen,  &QPushButton::clicked, this, &CfgVise::setRightVise);
  connect(ui->pbAddVise,    &QPushButton::clicked, this, &CfgVise::addVise);
  connect(ui->pbRemoveVise, &QPushButton::clicked, this, &CfgVise::delVise);
  connect(ui->lstVise->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CfgVise::viseSelected);
  }


void CfgVise::addVise() {
  ViseEntry* ve = new ViseEntry();

  ve->setName(ui->viseName->text());
  ve->setLeft(ui->leftVise->text());
  ve->setMiddle(ui->middleVise->text());
  ve->setRight(ui->rightVise->text());

  vModel->add(ve);

  ui->viseName->clear();
  ui->leftVise->clear();
  ui->middleVise->clear();
  ui->rightVise->clear();
  ui->viseName->setFocus();
  }


void CfgVise::delVise() {
  QModelIndex mi = ui->lstVise->currentIndex();

  vModel->removeRow(mi.row());
  }


void CfgVise::setLeftVise() {
  ui->leftVise->setText(Core().chooseCADFile());
  }


void CfgVise::setMidVise() {
  ui->middleVise->setText(Core().chooseCADFile());
  }


void CfgVise::setRightVise() {
  ui->rightVise->setText(Core().chooseCADFile());
  }


void CfgVise::viseSelected(const QItemSelection &selected, const QItemSelection &deselected) {
  QModelIndex   mi = ui->lstVise->currentIndex();
  ViseEntry*    ve = vModel->vise(mi.row());
  OcctQtViewer* view = Core().view3D();
  gp_Trsf       mL, mM, mR;
  Bnd_Box       bb;

  if (!leftVise.IsNull())  view->removeShape(leftVise);
  if (!midVise.IsNull())   view->removeShape(midVise);
  if (!rightVise.IsNull()) view->removeShape(rightVise);
  ui->viseName->setText(ve->name());
  ui->leftVise->setText(ve->left());
  ui->middleVise->setText(ve->middle());
  ui->rightVise->setText(ve->right());
  Core().loadVise(ve, leftVise, midVise, rightVise);

  mL.SetTranslation({0, 0, 0}, {-25, 0, 100});
  mM.SetTranslation({0, 0, 0}, {  0, 0, 100});
  mR.SetTranslation({0, 0, 0}, { 25, 0, 100});
  if (!leftVise.IsNull()) {
     leftVise->SetLocalTransformation(mL);
     bb.Add(leftVise->BoundingBox());
     if (midVise.IsNull()) leftVise->SetColor(Quantity_NOC_LIGHTSTEELBLUE1);
     else                  leftVise->SetColor(Quantity_NOC_ALICEBLUE);
     view->showShape(leftVise,  false);
     }
  if (!midVise.IsNull()) {
     midVise->SetLocalTransformation(mM);
     bb.Add(midVise->BoundingBox());
     midVise->SetColor(Quantity_NOC_LIGHTSTEELBLUE1);
     view->showShape(midVise,   false);
     }
  if (!rightVise.IsNull()) {
     rightVise->SetLocalTransformation(mR);
     bb.Add(rightVise->BoundingBox());
     rightVise->SetColor(Quantity_NOC_ALICEBLUE);
     view->showShape(rightVise, false);
     }
  bb.Update(0, 0, 170);
  qDebug() << "vise bounds:" << bb.CornerMin().X() << " / " << bb.CornerMin().Y() << " / " << bb.CornerMin().Z()
                 << "\tto\t" << bb.CornerMax().X() << " / " << bb.CornerMax().Y() << " / " << bb.CornerMax().Z();
  view->setBounds(bb);
  view->fitAll();
  view->refresh();
  }
