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
#include "kuteCAM.h"
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
  ui->tipDiameter->installEventFilter(this);
  ui->fluteLength->installEventFilter(this);
  ui->fluteDiameter->installEventFilter(this);
  ui->freeLength->installEventFilter(this);
  ui->cuttingDepth->installEventFilter(this);
  ui->cuttingAngle->installEventFilter(this);
  ui->shankDiameter->installEventFilter(this);
  ui->numFlutes->installEventFilter(this);
  }


void DimToolEditor::cbColletChanged(const QVariant& ndx) {
  toolEntry->setCollet(ui->cbCollet->currentIndex());
  }


bool DimToolEditor::eventFilter(QObject* o, QEvent* event) {
  if (event->type() == QEvent::KeyPress) {
     QKeyEvent* e = static_cast<QKeyEvent*>(event);

     switch (e->key()) {
       case Qt::Key_Tab: {
            QLineEdit* ed = static_cast<QLineEdit*>(o);
            if (o == ui->fluteLength)        toolEntry->setFluteLength(kute::textAsDouble(ed->text()));
            else if (o == ui->fluteDiameter) toolEntry->setFluteDiameter(kute::textAsDouble(ed->text()));
            else if (o == ui->tipDiameter)   toolEntry->setTipDiameter(kute::textAsDouble(ed->text()));
            else if (o == ui->freeLength)    toolEntry->setFreeLength(kute::textAsDouble(ed->text()));
            else if (o == ui->cuttingDepth)  toolEntry->setCuttingDepth(kute::textAsDouble(ed->text()));
            else if (o == ui->cuttingAngle)  toolEntry->setCuttingAngle(kute::textAsDouble(ed->text()));
            else if (o == ui->shankDiameter) toolEntry->setShankDiameter(kute::textAsDouble(ed->text()));
            else if (o == ui->numFlutes)     toolEntry->setNumFlutes(ed->text().toInt());
            } break;
       }
     }
  return false;
  }


void DimToolEditor::saveTool(ToolEntry* tool) {
  tool->setTipDiameter(kute::textAsDouble(ui->tipDiameter->text()));
  tool->setFluteLength(kute::textAsDouble(ui->fluteLength->text()));
  tool->setFluteDiameter(kute::textAsDouble(ui->fluteDiameter->text()));
  tool->setFreeLength(kute::textAsDouble(ui->freeLength->text()));
  tool->setCuttingDepth(kute::textAsDouble(ui->cuttingDepth->text()));
  tool->setCuttingAngle(kute::textAsDouble(ui->cuttingAngle->text()));
  tool->setShankDiameter(kute::textAsDouble(ui->shankDiameter->text()));
  tool->setNumFlutes(ui->numFlutes->text().toInt());
  }


void DimToolEditor::setTool(ToolEntry* toolEntry) {
  this->toolEntry = toolEntry;

  ui->tipDiameter->setText(QLocale::system().toString(toolEntry->tipDiameter(), 'f', 3));
  ui->fluteLength->setText(QLocale::system().toString(toolEntry->fluteLength(), 'f', 3));
  ui->fluteDiameter->setText(QLocale::system().toString(toolEntry->fluteDiameter(), 'f', 3));
  ui->freeLength->setText(QLocale::system().toString(toolEntry->freeLength(), 'f', 3));
  ui->cuttingDepth->setText(QLocale::system().toString(toolEntry->cuttingDepth(), 'f', 3));
  ui->cuttingAngle->setText(QLocale::system().toString(toolEntry->cuttingAngle(), 'f', 3));
  ui->shankDiameter->setText(QLocale::system().toString(toolEntry->shankDiameter(), 'f', 3));
  ui->numFlutes->setText(QString("%1").arg(toolEntry->numFlutes(), 0, 10));
  }
