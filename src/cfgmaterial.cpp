/* 
 * **************************************************************************
 * 
 *  file:       cfgmaterial.cpp
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
#include "cfgmaterial.h"
#include "ui_cfgMat.h"
#include "stringlistmodel.h"
#include <QKeyEvent>
#include <QSortFilterProxyModel>


CfgMaterial::CfgMaterial(StringListModel* matModel, QWidget *parent)
 : QWidget(parent)
 , ui(new Ui::MaterialConfig)
 , matModel(matModel) {
  ui->setupUi(this);
  matProxy = new QSortFilterProxyModel(this);
  matProxy->setSourceModel(matModel);
  matProxy->setDynamicSortFilter(true);
  matProxy->sort(0);
  ui->lstMaterial->setModel(matProxy);
  connect(ui->pbMatAdd,    &QPushButton::clicked, this, &CfgMaterial::addMaterial);
  connect(ui->pbMatRemove, &QPushButton::clicked, this, &CfgMaterial::delMaterial);
  connect(ui->lstMaterial->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CfgMaterial::materialSelected);
  ui->material->installEventFilter(this);
  }


void CfgMaterial::addMaterial() {
  if (!ui->material->text().isEmpty()) {
     matModel->add(ui->material->text());
     ui->material->clear();
     ui->material->setFocus();
     }
  }


void CfgMaterial::delMaterial() {
  const QModelIndex& srcIndex = matProxy->mapToSource(ui->lstMaterial->currentIndex());

  matModel->removeRow(srcIndex.row());
  }


bool CfgMaterial::eventFilter(QObject* o, QEvent* event) {
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


void CfgMaterial::materialSelected(const QItemSelection &selected, const QItemSelection &deselected) {
  QModelIndex     mi = ui->lstMaterial->currentIndex();
  const QVariant& v  = ui->lstMaterial->model()->data(mi);

  ui->material->setText(v.toString());
  }
