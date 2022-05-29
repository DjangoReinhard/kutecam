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
#include <QHideEvent>
#include <QShowEvent>


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
  connect(ui->toolName, &QLineEdit::textEdited, this, &ToolEditor::toolNameChanged);
  connect(ui->spToolNum, &QSpinBox::valueChanged, this, &ToolEditor::toolNumChanged);
  }


void ToolEditor::addTool(ToolEntry* nt) {
  QModelIndex mi   = ui->lstTools->currentIndex();
  ToolEntry*  tool = Core().toolListModel()->tool(mi.row());

  if (tool) saveTool(tool);
  Core().toolListModel()->add(nt);
  setTool(nt);
  }


bool ToolEditor::eventFilter(QObject* o, QEvent* event) {
  if (event->type() == QEvent::KeyPress) {
     QKeyEvent* e = static_cast<QKeyEvent*>(event);

     switch (e->key()) {
       case Qt::Key_Insert:
            int        rc = Core().toolListModel()->rowCount();
            ToolEntry* nt = new ToolEntry(rc);

            addTool(nt);
            return true;
       }
     }
  return false;
  }


void ToolEditor::initialize() {
  ui->spToolNum->setValue(curTool->toolNumber());
  ui->toolName->setText(curTool->toolName());
  edDim = new DimToolEditor(curTool);
  edCP  = new CutParmToolEditor(matModel, curTool);
  ui->notebook->addTab(edDim, tr("Properties"));
  ui->notebook->addTab(edCP, tr("Cutting Parameters"));
  edDim->installEventFilter(this);
  edCP->installEventFilter(this);
  ui->lstTools->installEventFilter(this);
  installEventFilter(this);
  }


void ToolEditor::hideEvent(QHideEvent *event) {
  emit teActivated(false);
  }


void ToolEditor::showEvent(QShowEvent *event) {
  emit teActivated(true);
  }


void ToolEditor::toolNameChanged(const QString& name) {
  if (!curTool) return;
  curTool->setToolName(name);
  }


void ToolEditor::toolNumChanged(int num) {
  if (!curTool) return;
  curTool->setToolNumber(num);
  }


void ToolEditor::toolSelected(const QItemSelection& selected, const QItemSelection& ) {
  QModelIndexList il  = selected.indexes();
  int             mx  = 0;

  if (il.count()) mx = il.at(0).row();
  curTool = tools->tool(mx);
  ui->spToolNum->setValue(curTool->toolNumber());
  ui->toolName->setText(curTool->toolName());
  edDim->setTool(curTool);
  edCP->setTool(curTool);
  }


void ToolEditor::saveTool(ToolEntry* tool) {
  tool->setToolNumber(ui->spToolNum->value());
  tool->setToolName(ui->toolName->text());
  edDim->saveTool(tool);
  edCP->saveTool(tool);
  }


void ToolEditor::setTool(ToolEntry *tool) {
  if (!tool) return;
  curTool = tool;

  ui->spToolNum->setValue(curTool->toolNumber());
  ui->toolName->setText(curTool->toolName());
  edDim->setTool(curTool);
  edCP->setTool(curTool);
  ui->toolName->setFocus();
  }
