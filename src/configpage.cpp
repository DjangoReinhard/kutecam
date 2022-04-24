/* 
 * **************************************************************************
 * 
 *  file:       configpage.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    11.4.2022 by Django Reinhard
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
#include "configpage.h"
#include "ui_misc.h"
#include "core.h"
#include "stringlistmodel.h"
#include "occtviewer.h"
#include "viseentry.h"
#include "viselistmodel.h"
#include <QFileDialog>
#include <QDir>
#include <QKeyEvent>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QDebug>


ConfigPage::ConfigPage(StringListModel* matModel, ViseListModel* vModel, QWidget *parent)
 : ApplicationWindow(parent)
 , ui(new Ui::MiscPage)
 , vModel(vModel)
 , matModel(matModel) {
  ui->setupUi(this);

  connect(ui->pbLeftOpen,  &QPushButton::clicked, this, &ConfigPage::setLeftVise);
  connect(ui->pbMidOpen,   &QPushButton::clicked, this, &ConfigPage::setMidVise);
  connect(ui->pbRightOpen, &QPushButton::clicked, this, &ConfigPage::setRightVise);

  connect(ui->pbMatAdd,    &QPushButton::clicked, this, &ConfigPage::addMaterial);
  connect(ui->pbMatRemove, &QPushButton::clicked, this, &ConfigPage::delMaterial);

  connect(ui->pbAddVise,    &QPushButton::clicked, this, &ConfigPage::addVise);
  connect(ui->pbRemoveVise, &QPushButton::clicked, this, &ConfigPage::delVise);    

  QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);

  proxyModel->setSourceModel(matModel);
  proxyModel->setDynamicSortFilter(true);
  proxyModel->sort(0);
  ui->lstMaterial->setModel(proxyModel);
  ui->lstVise->setModel(vModel);
  connect(ui->lstVise->selectionModel(),     &QItemSelectionModel::selectionChanged, this, &ConfigPage::viseSelected);
  connect(ui->lstMaterial->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ConfigPage::materialSelected);
  ui->material->installEventFilter(this);
  }


void ConfigPage::addMaterial() {
  if (!ui->material->text().isEmpty()) {
     matModel->add(ui->material->text());
     ui->material->clear();
     ui->material->setFocus();
     }
  }


void ConfigPage::addVise() {
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


void ConfigPage::closeEvent(QCloseEvent *e) {
  qDebug() << "config-page closeEvent ...";
  QSettings& cfg = Core().cfg();
  int mx = matModel->rowCount();

  cfg.beginGroup("Work");
  cfg.beginWriteArray("Materials");

  qDebug() << "write out " << mx << "materials";
  for (int i=0; i < mx; ++i) {
      cfg.setArrayIndex(i);
      cfg.setValue("Name", matModel->data(matModel->index(i, 0)));
      }
  cfg.endArray();
  cfg.setValue("A-is-table", ui->cAisTable->isChecked());
  cfg.setValue("B-is-table", ui->cBisTable->isChecked());
  cfg.setValue("C-is-table", ui->cCisTable->isChecked());
  cfg.beginWriteArray("Vises");
  mx = vModel->rowCount();
  ViseEntry* ve;

  qDebug() << "write out " << mx << "vises";
  for (int i=0; i < mx; ++i) {
      ve = vModel->vise(i);

      cfg.setArrayIndex(i);
      cfg.setValue("name",   ve->name());
      cfg.setValue("left",   ve->left());
      cfg.setValue("middle", ve->middle());
      cfg.setValue("right",  ve->right());
      }
  cfg.endArray();
  cfg.endGroup();
  cfg.sync();
  }


void ConfigPage::delMaterial() {
  QModelIndex mi = ui->lstMaterial->currentIndex();

  ui->lstMaterial->model()->removeRow(mi.row());
  }


void ConfigPage::delVise() {
  QModelIndex mi = ui->lstVise->currentIndex();

  vModel->removeRow(mi.row());
  }


bool ConfigPage::eventFilter(QObject* o, QEvent* event) {
  if (event->type() == QEvent::KeyPress) {
     QKeyEvent* e = static_cast<QKeyEvent*>(event);

     switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
            if (o == ui->material) {
               ui->pbMatAdd->click();
               return true;
               }
       }
     }
  return false;
  }


void ConfigPage::materialSelected(const QItemSelection &selected, const QItemSelection &deselected) {
  QModelIndex     mi = ui->lstMaterial->currentIndex();
  const QVariant& v  = ui->lstMaterial->model()->data(mi);

  ui->material->setText(v.toString());
  }


void ConfigPage::setLeftVise() {
  ui->leftVise->setText(Core().chooseCADFile());
  }


void ConfigPage::setMidVise() {
  ui->middleVise->setText(Core().chooseCADFile());
  }


void ConfigPage::setRightVise() {
  ui->rightVise->setText(Core().chooseCADFile());
  }


void ConfigPage::showEvent(QShowEvent *event) {
  if (ui && ui->material) ui->material->setFocus();
  }


void ConfigPage::viseSelected(const QItemSelection &selected, const QItemSelection &deselected) {
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
