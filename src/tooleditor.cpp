/* 
 * **************************************************************************
 * 
 *  file:       tooleditor.cpp
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
#include "tooleditor.h"
#include "toolentry.h"
#include "cutparamlistmodel.h"
#include "cutparmtooleditor.h"
#include "cuttingparameters.h"
#include "core.h"
#include "dimtooleditor.h"
#include "stringlistmodel.h"
#include "toollistmodel.h"
#include "ui_tool.h"
#include "toollistmodel.h"
#include <cmath>
#include <QDebug>
#include <QInputDialog>


ToolEditor::ToolEditor(StringListModel* matModel, ToolListModel* tools, QWidget *parent)
 : QWidget(parent)
 , ui(new Ui::ToolEditor)
 , toolEntry(0, "unknown")
 , model(new CutParamListModel())
 , matModel(matModel)
 , curTool(&toolEntry)
 , curCutParams(nullptr)
 , tools(tools)
 , edDim(nullptr)
 , edCP(nullptr) {
  ui->setupUi(this);
  ui->lstTools->setModel(tools);
  connect(ui->lstTools->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ToolEditor::toolSelected);
  }


void ToolEditor::initialize() {
  ui->spToolNum->setValue(curTool->toolNumber());
  ui->toolName->setText(curTool->toolName());
  edDim = new DimToolEditor(curTool);
  edCP  = new CutParmToolEditor(matModel, curTool);
  ui->notebook->addTab(edDim, tr("Properties"));
  ui->notebook->addTab(edCP, tr("Cutting Parameters"));
  }


double ToolEditor::readDouble(QLineEdit* edit) {
  QString userInput = edit->text();
  bool    ok;

  userInput.replace(',', '.');
  double rv = userInput.toDouble(&ok);

  return ok ? rv : 0;
  }


void ToolEditor::toolSelected(const QItemSelection& selected, const QItemSelection& ) {
//  QModelIndexList dil = deselected.indexes();
  QModelIndexList il  = selected.indexes();
  int             mx  = 0;

  if (il.count()) mx = il.at(0).row();
  curTool = tools->tool(mx);
  ui->spToolNum->setValue(curTool->toolNumber());
  ui->toolName->setText(curTool->toolName());
  edDim->setTool(curTool);
  edCP->setTool(curTool);
  }


#ifdef REDNOSE
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
#endif
