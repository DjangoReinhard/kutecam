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
#include "ui_mainwindow.h"
#include "cfggeneral.h"
#include "cfgmaterial.h"
#include "cfgvise.h"
#include "core.h"
#include "kuteCAM.h"
#include "mainwindow.h"
#include "stringlistmodel.h"
#include "occtviewer.h"
#include "preview3d.h"
#include "projectfile.h"
#include "tooleditor.h"
#include "toollistmodel.h"
#include "viseentry.h"
#include "viselistmodel.h"
#include "xmltoolwriter.h"
#include <QAction>
#include <QFileDialog>
#include <QDir>
#include <QKeyEvent>
#include <QLabel>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QToolBox>
#include <QDebug>


ConfigPage::ConfigPage(StringListModel* matModel, ViseListModel* vises, QWidget *parent)
 : ApplicationWindow(parent)
 , pages(nullptr)
 , tools(nullptr)
 , matModel(matModel)
 , vises(vises) {
  QVBoxLayout* l = new QVBoxLayout(this);

  pages = new QToolBox;
  l->addWidget(pages);
  setLayout(l);
  connect(Core().uiMainWin()->actionToolsOpen, &QAction::triggered, this, &ConfigPage::loadTools);
  connect(Core().uiMainWin()->actionToolsSave, &QAction::triggered, this, &ConfigPage::saveTools);
  }


void ConfigPage::closeEvent(QCloseEvent *e) {
  // GCode, Material and vises are loaded by kernel,
  // but we have to store them!
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
  cfg.setValue("PP", Core().postProcessor());
  cfg.setValue("A-is-table", Core().isAAxisTable());
  cfg.setValue("B-is-table", Core().isBAxisTable());
  cfg.setValue("C-is-table", Core().isCAxisTable());
  cfg.setValue("opAllInOne", Core().isAllInOneOperation());
  cfg.setValue("autoRotateSelected", Core().autoRotateSelection());
  cfg.setValue("machineType", Core().machineType());
  cfg.setValue("genSepToolChange", Core().isSepWithToolChange());
  cfg.beginWriteArray("Vises");
  mx = vises->rowCount();
  ViseEntry* ve;

  qDebug() << "write out " << mx << "vises";
  for (int i=0; i < mx; ++i) {
      ve = vises->vise(i);

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


void ConfigPage::initialize() {
  tools = Core().toolListModel();
  ToolEditor* te = new ToolEditor(matModel, tools, this);

  te->initialize();
  connect(te, &ToolEditor::teActivated, Core().mainWin()->preview3D(), &Preview3D::toolHint);
  pages->addItem(new CfgGeneral(this), tr("General Settings"));
  pages->addItem(new CfgMaterial(matModel, this), tr("Material"));
  pages->addItem(te, tr("Tools"));
  pages->addItem(new CfgVise(vises, this), tr("Vises"));
  }


void ConfigPage::loadTools() {
  QString     fileName;
  QFileDialog dialog(this
                   , tr("QFileDialog::getOpenFileName()")
                   , kute::BasePath
                   , tr("XML-Documents (*.xml)"));

  dialog.setSupportedSchemes(QStringList(QStringLiteral("file")));
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setOption(QFileDialog::DontUseNativeDialog);

  if (dialog.exec() != QDialog::Accepted) return;
  fileName = dialog.selectedUrls().value(0).toLocalFile();

  if (Core().loadTools(fileName)) {
     ProjectFile* pf = Core().projectFile();

     if (pf) {
        pf->beginGroup("Setup");
        pf->setValue("Tool-file", fileName);
        pf->endGroup();
        }
     }
  }


void ConfigPage::saveTools() {
  QString     fileName;
  QFileDialog dialog(this
                   , tr("QFileDialog::getSaveFileName()")
                   , kute::BasePath
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
