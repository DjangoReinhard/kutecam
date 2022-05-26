/* 
 * **************************************************************************
 * 
 *  file:       clipdialog.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    26.5.2022 by Django Reinhard
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
#include "clipdialog.h"
#include "ui_clip.h"
#include <QDebug>


ClipDialog::ClipDialog(const gp_Pnt& pos, const gp_Dir& dir, QWidget* parent)
 : QDialog(parent)
 , ui(new Ui::ClipDialog) {
  ui->setupUi(this);
  ui->spXPos->setValue(pos.X());
  ui->spYPos->setValue(pos.Y());
  ui->spZPos->setValue(pos.Z());
  ui->spXDir->setValue(dir.X());
  ui->spYDir->setValue(dir.Y());
  ui->spZDir->setValue(dir.Z());
  setWindowTitle(tr("modify clip parameters"));
  connect(ui->pbApply, &QPushButton::clicked, this, &QDialog::accept);
  }


bool ClipDialog::clippingEnabled() const {
  return ui->cClipping->isChecked();
  }


gp_Pnt ClipDialog::clipPosition() const {
  double x = ui->spXPos->value();
  double y = ui->spYPos->value();
  double z = ui->spZPos->value();

  return gp_Pnt(x, y, z);
  }


gp_Dir ClipDialog::clipDirection() const {
  double x = ui->spXDir->value();
  double y = ui->spYDir->value();
  double z = ui->spZDir->value();

  return gp_Dir(x, y, z);
  }


void ClipDialog::showEvent(QShowEvent *event) {
  QWidget* p = qobject_cast<QWidget*>(parent());
  QSize    ps;
  QSize    ts = size();

  if (p) ps = p->size();

  qDebug() << "my size is " << ts.width() << "x" << ts.height();
  qDebug() << "parents size is " << ps.width() << "x" << ps.height();
  }
