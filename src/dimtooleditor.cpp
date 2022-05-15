/* 
 * **************************************************************************
 * 
 *  file:       dimtooleditor.cpp
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
#include "dimtooleditor.h"
#include "ui_tDim.h"
#include "toolentry.h"
#include <QKeyEvent>


DimToolEditor::DimToolEditor(ToolEntry* toolEntry, QWidget *parent)
 : QWidget(parent)
 , ui(new Ui::DimensionToolEditor())
 , toolEntry(toolEntry) {
  ui->setupUi(this);
  ui->cbCollet->addItem("ER40");
  ui->cbCollet->addItem("ER32");
  ui->cbCollet->addItem("ER25");
  ui->cbCollet->addItem("ER20");

  connect(ui->cbCollet, &QComboBox::currentIndexChanged, this, &DimToolEditor::cbColletChanged);
  ui->fluteLength->installEventFilter(this);
  ui->fluteDiameter->installEventFilter(this);
  ui->freeLength->installEventFilter(this);
  ui->cuttingDepth->installEventFilter(this);
  ui->shankDiameter->installEventFilter(this);
  ui->numFlutes->installEventFilter(this);
  }


void DimToolEditor::cbColletChanged(const QVariant& ndx) {
//  int index = ndx.toInt();

  toolEntry->setCollet(ui->cbCollet->currentIndex());
  }


bool DimToolEditor::eventFilter(QObject* o, QEvent* event) {
  if (event->type() == QEvent::KeyPress) {
     QKeyEvent* e = static_cast<QKeyEvent*>(event);

     switch (e->key()) {
       case Qt::Key_Tab: {
            QLineEdit* ed = static_cast<QLineEdit*>(o);
            bool       ok = false;
            double     v  = ed->text().toDouble(&ok);

            if (!ok) return false;
            if (o == ui->fluteLength) {
               toolEntry->setFluteLength(v);
               return true;
               }
            else if (o == ui->fluteDiameter) {
               toolEntry->setFluteDiameter(v);
               return true;
               }
            else if (o == ui->freeLength) {
               toolEntry->setFreeLength(v);
               return true;
               }
            else if (o == ui->cuttingDepth) {
               toolEntry->setCuttingDepth(v);
               return true;
               }
            else if (o == ui->shankDiameter) {
               toolEntry->setShankDiameter(v);
               return true;
               }
            else if (o == ui->numFlutes) {
               toolEntry->setNumFlutes((int) v);
               return true;
               }
            } break;
       }
     }
  return false;
  }


void DimToolEditor::setTool(ToolEntry* toolEntry) {
  this->toolEntry = toolEntry;

  ui->fluteLength->setText(QString("%1").arg(toolEntry->fluteLength(), 0, 'f', 3));
  ui->fluteDiameter->setText(QString("%1").arg(toolEntry->fluteDiameter(), 0, 'f', 3));
  ui->freeLength->setText(QString("%1").arg(toolEntry->freeLength(), 0, 'f', 3));
  ui->cuttingDepth->setText(QString("%1").arg(toolEntry->cuttingDepth(), 0, 'f', 3));
  ui->shankDiameter->setText(QString("%1").arg(toolEntry->shankDiameter(), 0, 'f', 3));
  ui->numFlutes->setText(QString("%1").arg(toolEntry->numFlutes(), 0, 10));
  }
