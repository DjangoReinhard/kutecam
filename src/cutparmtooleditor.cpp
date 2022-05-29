/* 
 * **************************************************************************
 * 
 *  file:       cutparmtooleditor.cpp
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
#include "cutparmtooleditor.h"
#include "ui_tCutParm.h"
#include "core.h"
#include "cutparamlistmodel.h"
#include "cuttingparameters.h"
#include "stringlistmodel.h"
#include "toolentry.h"
#include "toollistmodel.h"
#include <QInputDialog>
#include <QDebug>


CutParmToolEditor::CutParmToolEditor(StringListModel* matModel, ToolEntry* toolEntry, QWidget *parent)
 : QWidget(parent)
 , ui(new Ui::CuttingParameters)
 , model(new CutParamListModel())
 , matModel(matModel)
 , curCutParams(nullptr)
 , toolEntry(toolEntry) {
  ui->setupUi(this);
  ui->lstCutParams->setModel(model);
  ui->lstCutParams->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(ui->cuttingSpeed, &QDoubleSpinBox::valueChanged, this, &CutParmToolEditor::speedChanged);
  connect(ui->toothFeed,    &QDoubleSpinBox::valueChanged, this, &CutParmToolEditor::feedChanged);
  connect(ui->widthOfCut,   &QDoubleSpinBox::valueChanged, this, &CutParmToolEditor::wocChanged);
  connect(ui->depthOfCut,   &QDoubleSpinBox::valueChanged, this, &CutParmToolEditor::docChanged);
  connect(ui->pbAdd,        &QPushButton::clicked,         this, &CutParmToolEditor::addMaterial);
  connect(ui->pbDel,        &QPushButton::clicked,         this, &CutParmToolEditor::delMaterial);
  connect(ui->lstCutParams->selectionModel(),  &QItemSelectionModel::selectionChanged, this, &CutParmToolEditor::cutParamChanged);
  }


void CutParmToolEditor::addMaterial() {
  bool    ok;
  QString material = QInputDialog::getItem(this
                                         , tr("QInputDialog::getItem()")
                                         , tr("Material:")
                                         , matModel->stringList()
                                         , 0
                                         , false
                                         , &ok);

  if (ok && !material.isEmpty()) {
     toolEntry->cutParameters().append(new CuttingParameters(material));
     model->replace(toolEntry->cutParameters());
     ui->lstCutParams->setCurrentIndex(model->index(0, 0));
     }
  }


void CutParmToolEditor::calcSpeed() {
  double vc   = ui->cuttingSpeed->value();
  double feed = ui->toothFeed->value();
  double s    = vc * 1000 / M_PI / toolEntry->fluteDiameter();
  double f    = s * toolEntry->numFlutes() * feed;

  ui->resFeed->setText(QString("F%1").arg(f, 0, 'f', 0));
  ui->resSpeed->setText(QString("S%1").arg(s, 0, 'f', 0));
  }


void CutParmToolEditor::cutParamChanged(const QItemSelection &selected, const QItemSelection &deselected) {
  if (!toolEntry) return;
  QModelIndexList il = selected.indexes();
  QModelIndex     mi = il.at(0);

  qDebug() << "current material changed: #" << mi.row();

  curCutParams = toolEntry->cutParameter(mi.row());

  ui->cuttingSpeed->setValue(curCutParams->cuttingSpeed());
  ui->toothFeed->setValue(curCutParams->toothFeed());
  ui->widthOfCut->setValue(curCutParams->widthOfCut());
  ui->depthOfCut->setValue(curCutParams->depthOfCut());

  calcSpeed();
  }


void CutParmToolEditor::delMaterial() {
  //TODO:
  }


void CutParmToolEditor::docChanged(const QVariant& v) {
  if (!curCutParams) return;
  curCutParams->setDepthOfCut(v.toDouble());
  }


void CutParmToolEditor::feedChanged(const QVariant& f) {
  if (!curCutParams) return;
  curCutParams->setToothFeed(f.toDouble());
  calcSpeed();
  }


void CutParmToolEditor::saveTool(ToolEntry* toolEntry) {

  }


void CutParmToolEditor::setTool(ToolEntry *toolEntry) {
  this->toolEntry = toolEntry;
  model->replace(toolEntry->cutParameters());
  ui->lstCutParams->setCurrentIndex(model->index(0, 0));
  }


void CutParmToolEditor::speedChanged(const QVariant& v) {
  if (!curCutParams) return;
  curCutParams->setCuttingSpeed(v.toDouble());
  calcSpeed();
  }


void CutParmToolEditor::wocChanged(const QVariant& v) {
  if (!curCutParams) return;
  curCutParams->setWidthOfCut(v.toDouble());
  }
