/* 
 * **************************************************************************
 * 
 *  file:       toolpage.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    4.4.2022 by Django Reinhard
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
#include "toolpage.h"
#include "ui_mainwindow.h"
#include "occtviewer.h"
#include "core.h"
#include "stringlistmodel.h"
#include "toolentry.h"
#include "toollistmodel.h"
#include "tooleditor.h"
#include "projectfile.h"
#include "xmltoolreader.h"
#include "xmltoolwriter.h"
#include <QFileDialog>
#include <QKeyEvent>
#include <QLayout>
#include <QListView>
#include <QSplitter>
#include <QDebug>


ToolPage::ToolPage(StringListModel* matModel, QWidget *parent)
 : ApplicationWindow(parent)
 , tools(new QListView)
 , edit(new ToolEditor(matModel))
 , matModel(matModel) {
  QGridLayout* gl = new QGridLayout;

  setContentsMargins(0, 0, 0, 0);
  qDebug() << "tool model has" << Core().toolListModel()->rowCount() << "rows";
  tools->setSelectionMode(QAbstractItemView::SingleSelection);
  tools->setModel(Core().toolListModel());
  tools->update();
  gl->addWidget(tools, 0, 0);
  gl->addWidget(edit, 1, 0);
  gl->setContentsMargins(0, 0, 0, 0);
  gl->setRowStretch(0, 1);
  setLayout(gl);
  tools->installEventFilter(this);
  edit->installEventFilter(this);
  installEventFilter(this);

  connect(tools->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ToolPage::toolSelected);
  connect(Core().uiMainWin()->actionToolsOpen, &QAction::triggered, this, &ToolPage::loadTools);
  connect(Core().uiMainWin()->actionToolsSave, &QAction::triggered, this, &ToolPage::saveTools);
  }


bool ToolPage::eventFilter(QObject* o, QEvent* event) {
  if (event->type() == QEvent::KeyPress) {
     QKeyEvent* e = static_cast<QKeyEvent*>(event);

     switch (e->key()) {
       case Qt::Key_Insert:
            int        rc = Core().toolListModel()->rowCount() + 1;
            ToolEntry* nt = new ToolEntry(rc);

            addTool(nt);
            return true;
       }
     }
  return false;
  }


void ToolPage::addTool(ToolEntry* nt) {
  QModelIndex mi   = tools->currentIndex();
  ToolEntry*  tool = Core().toolListModel()->tool(mi.row());

  if (tool) edit->saveTool(tool);
  Core().toolListModel()->add(nt);
  edit->setTool(nt);
  }


void ToolPage::loadTools() {
  QString     fileName;
  QFileDialog dialog(this
                   , tr("QFileDialog::getOpenFileName()")
                   , "/media/Scratch"
                   , tr("XML-Documents (*.xml)"));

  dialog.setSupportedSchemes(QStringList(QStringLiteral("file")));
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setOption(QFileDialog::DontUseNativeDialog);

  if (dialog.exec() != QDialog::Accepted) return;
  fileName = dialog.selectedUrls().value(0).toLocalFile();

  if (readTools(fileName)) {
     ProjectFile* pf = Core().projectFile();

     if (pf) {
        pf->beginGroup("Setup");
        pf->setValue("Tool-file", fileName);
        pf->endGroup();
        }
     }
  }


bool ToolPage::readTools(const QString &fileName) {
  QFile inFile(fileName);
  XmlToolReader xtr;

  if (inFile.exists()) {
     Core().toolListModel()->setData(xtr.read(&inFile));
     inFile.close();

     return true;
     }
  return false;
  }


void ToolPage::saveTools() {
  QString     fileName;
  QFileDialog dialog(this
                   , tr("QFileDialog::getSaveFileName()")
                   , "/media/Scratch"
                   , tr("XML-Documents (*.xml)"));

  dialog.setSupportedSchemes(QStringList(QStringLiteral("file")));
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted) return;
  fileName = dialog.selectedUrls().value(0).toLocalFile();
  if (!fileName.endsWith(".xml")) fileName += ".xml";

  qDebug() << "save tools to:" << fileName;

  XmlToolWriter xtw;

  xtw.write(fileName, Core().toolListModel()->tools());
  }


void ToolPage::toolSelected(const QItemSelection &selected, const QItemSelection &deselected) {
  QModelIndexList dil = deselected.indexes();
  QModelIndexList il  = selected.indexes();
  int             mx  = dil.count();

  for (int i=0; i < mx; ++i) {
      QModelIndex mi   = dil.at(i);
      ToolEntry*  tool = Core().toolListModel()->tool(mi.row());

      edit->saveTool(tool);
      }
  mx = il.count();

  for (int i=0; i < mx; ++i) {
      QModelIndex mi   = il.at(i);
      ToolEntry*  tool = Core().toolListModel()->tool(mi.row());

      edit->setTool(tool);
      }
  }
