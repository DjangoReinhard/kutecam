/* 
 * **************************************************************************
 * 
 *  file:       tooleditor.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    11.4.2022 by Django Reinhard
 *  copyright:  2022 - 2022 Django Reinhard -  all rights reserved
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
#include "tooleditor.h"
#include "toolentry.h"
#include "cutparamlistmodel.h"
#include "cuttingparameters.h"
#include "core.h"
#include "stringlistmodel.h"
#include "toollistmodel.h"
#include "ui_tool.h"
#include <cmath>
#include <QDebug>
#include <QInputDialog>


ToolEditor::ToolEditor(StringListModel* matModel, QWidget *parent)
 : QWidget(parent)
 , ui(new Ui::ToolEditor)
 , model(new CutParamListModel())
 , matModel(matModel)
 , curTool(nullptr)
 , curCutParams(nullptr) {
  ui->setupUi(this);

  ui->cbCollet->addItem("ER40");
  ui->cbCollet->addItem("ER32");
  ui->cbCollet->addItem("ER25");
  ui->cbCollet->addItem("ER20");

  ui->lstCutParams->setModel(model);
  ui->lstCutParams->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(ui->cuttingSpeed, &QDoubleSpinBox::valueChanged, this, &ToolEditor::speedChanged);
  connect(ui->toothFeed,    &QDoubleSpinBox::valueChanged, this, &ToolEditor::feedChanged);
  connect(ui->widthOfCut,   &QDoubleSpinBox::valueChanged, this, &ToolEditor::wocChanged);
  connect(ui->depthOfCut,   &QDoubleSpinBox::valueChanged, this, &ToolEditor::docChanged);
  connect(ui->pbAdd,        &QPushButton::clicked,         this, &ToolEditor::addMaterial);
  connect(ui->pbDel,        &QPushButton::clicked,         this, &ToolEditor::delMaterial);
  connect(ui->lstCutParams->selectionModel(),  &QItemSelectionModel::selectionChanged, this, &ToolEditor::cutParamChanged);
  }


void ToolEditor::addMaterial() {
  if (curTool) saveTool(curTool);
  bool    ok;
  QString material = QInputDialog::getItem(this
                                         , tr("QInputDialog::getItem()")
                                         , tr("Material:")
                                         , matModel->stringList()
                                         , 0
                                         , false
                                         , &ok);

  if (ok && !material.isEmpty()) {
     QVector<ToolEntry*> tools = Core().toolListModel()->tools();

     for (ToolEntry* te : tools) {
         if (!te->hasMaterial(material))
            te->cutParameters().append(new CuttingParameters(material));
         }
     model->replace(curTool->cutParameters());
     ui->lstCutParams->setCurrentIndex(model->index(0, 0));
     }
  }


void ToolEditor::calcSpeed() {
  double vc   = ui->cuttingSpeed->value();
  double feed = ui->toothFeed->value();
  double diam = ui->fluteDiameter->text().toDouble();
  int    num  = ui->numFlutes->text().toInt();

  double s = vc * 1000 / M_PI / diam;
  double f = s * num * feed;

  ui->resFeed->setText(QString("F%1").arg(f, 0, 'f', 0));
  ui->resSpeed->setText(QString("S%1").arg(s, 0, 'f', 0));
  }


void ToolEditor::cutParamChanged(const QItemSelection &selected, const QItemSelection &deselected) {
  if (!curTool) return;
  QModelIndexList il = selected.indexes();
  QModelIndex     mi = il.at(0);

  qDebug() << "current material changed: #" << mi.row();

  curCutParams = curTool->cutParameter(mi.row());

  ui->cuttingSpeed->setValue(curCutParams->cuttingSpeed());
  ui->toothFeed->setValue(curCutParams->toothFeed());
  ui->widthOfCut->setValue(curCutParams->widthOfCut());
  ui->depthOfCut->setValue(curCutParams->depthOfCut());

  calcSpeed();
  }


void ToolEditor::delMaterial() {
  //TODO:
  }


void ToolEditor::docChanged(const QVariant& v) {
  if (!curCutParams) return;
  curCutParams->setDepthOfCut(v.toDouble());
  }


void ToolEditor::feedChanged(const QVariant& f) {
  if (!curCutParams) return;
  curCutParams->setToothFeed(f.toDouble());
  calcSpeed();
  }


double ToolEditor::readDouble(QLineEdit* edit) {
  QString userInput = edit->text();
  bool    ok;

  userInput.replace(',', '.');
  double rv = userInput.toDouble(&ok);

  return ok ? rv : 0;
  }


void ToolEditor::saveTool(ToolEntry* tool) {
  tool->setCollet(ui->cbCollet->currentIndex());
  tool->setToolName(ui->toolName->text());
  tool->setFluteDiameter(readDouble(ui->fluteDiameter));
  tool->setFluteLength(readDouble(ui->fluteLength));
  tool->setCuttingDepth(readDouble(ui->cuttingDepth));
  tool->setShankDiameter(readDouble(ui->shankDiameter));
  tool->setFreeLength(readDouble(ui->freeLength));
  tool->setNumFlutes(ui->numFlutes->text().toInt());
  }


void ToolEditor::setTool(ToolEntry *tool) {
  if (!tool) return;

  curTool = tool;
  ui->cbCollet->setCurrentIndex(tool->collet());
  ui->toolNum->setText(QString("%1").arg(tool->toolNumber()));
  ui->toolName->setText(tool->toolName());
  ui->fluteDiameter->setText(QString("%1").arg(tool->fluteDiameter()));
  ui->fluteLength->setText(QString("%1").arg(tool->fluteLength()));
  ui->cuttingDepth->setText(QString("%1").arg(tool->cuttingDepth()));
  ui->shankDiameter->setText(QString("%1").arg(tool->shankDiameter()));
  ui->freeLength->setText(QString("%1").arg(tool->freeLength()));
  ui->numFlutes->setText(QString("%1").arg(tool->numFlutes()));
  model->replace(tool->cutParameters());
  ui->lstCutParams->setCurrentIndex(model->index(0, 0));
  ui->toolName->setFocus();
  }


void ToolEditor::speedChanged(const QVariant& v) {
  assert(curCutParams);
  curCutParams->setCuttingSpeed(v.toDouble());
  calcSpeed();
  }


void ToolEditor::wocChanged(const QVariant& v) {
  if (!curCutParams) return;
  curCutParams->setWidthOfCut(v.toDouble());
  }
