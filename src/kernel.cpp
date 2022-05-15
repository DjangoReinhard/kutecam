/* 
 * **************************************************************************
 * 
 *  file:       kernel.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    18.4.2022 by Django Reinhard
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
#include "kernel.h"
#include "core.h"
#include "projectfile.h"
#include "work.h"
#include "mainwindow.h"
#include "configpage.h"
#include "setuppage.h"
#include "util3d.h"
#include "occtviewer.h"
#include "geomlistmodel.h"
#include "setuppage.h"
#include "operationspage.h"
#include "preview3d.h"
#include "selectionhandler.h"
#include "shapelistmodel.h"
#include "tdfactory.h"
#include "toollistmodel.h"
#include "toolpage.h"
#include "viseentry.h"
#include "viselistmodel.h"
#include "wsfactory.h"
#include <BRepLib.hxx>
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QMap>
#include <QSplitter>


Kernel::Kernel(QApplication& app, MainWindow& win)
 : QObject(nullptr)
 , app(app)
 , win(win)
 , configData(QSettings::UserScope, "SRD", app.applicationName())
 , helper(nullptr)
 , selHdr(nullptr)
 , pf(nullptr)
 , work(nullptr)
 , operations(nullptr)
 , config(nullptr)
 , setupPage(nullptr)
 , tdFactory(new TDFactory)
 , wsFactory(new WSFactory)
 , matModel(new StringListModel(QStringList()))
 , geomListModel(new GeomListModel(this))
 , shapeListModel(new ShapeListModel(this))
 , viseListModel(new ViseListModel(this))
 , toolListModel(new ToolListModel(this)) {
  BRepLib::Precision(1e-4);
  }


void Kernel::clearCurves() {
  shapeListModel->clear();
  }


void Kernel::initialize() {
  win.initialize();
  loadConfig();
  view3D     = win.viewer3D();
  config     = new ConfigPage(matModel, viseListModel);
  setupPage  = new SetupPage(matModel, viseListModel);
  operations = new OperationsPage();
  helper     = new Util3D();
  selHdr     = new SelectionHandler();
  work       = new Work();

  config->initialize();

  pages[Core::PgWorkPiece]  = setupPage;
  pages[Core::PgConfig]     = config;
  pages[Core::PgOperations] = operations;

  win.setWindowTitle(QString("- %1 -").arg(app.applicationName()));
  win.addPage(setupPage);
  win.addPage(operations);
  win.addPage(config);
  win.restore();

  connect(view3D, &OcctQtViewer::clearCurves, this, &Kernel::clearCurves);
  configData.setValue("what", "nope");
  }


bool Kernel::loadConfig() {
  bool rv = loadMaterials();

  configData.beginGroup("Work");
  AisTable = configData.value("A-is-table").toBool();
  BisTable = configData.value("B-is-table").toBool();
  CisTable = configData.value("C-is-table").toBool();
  opAllInOne = configData.value("opAllInOne").toBool();
  genSepWithToolChange = configData.value("genSepToolChange").toBool();
  configData.endGroup();
  if (rv) rv = loadViseList();

  return rv;
  }


bool Kernel::loadModelFile(const QString &fileName) {
  win.setWindowTitle(QString("- %1 -- %2 -").arg(app.applicationName(), fileName));

  if (fileName.endsWith(".brep"))     topShape = helper->loadBRep(fileName);
  else if (fileName.endsWith(".step")
        || fileName.endsWith(".stp")) topShape = helper->loadStep(fileName);
  setupPage->setModel(topShape);
  operations->clear();
  pf = new ProjectFile();
  pf->beginGroup("Setup");
  pf->setValue("Model-File", fileName);
  pf->endGroup();

  qDebug() << "project-file is" << pf->fileName();

  return true;
  }


bool Kernel::loadMaterials() {
  configData.beginGroup("Work");
  int     size = configData.beginReadArray("Materials");
  bool    rv   = false;
  QString name;

  for (int i = 0; i < size; ++i) {
      configData.setArrayIndex(i);
      name = configData.value("Name").toString();
      matModel->add(name);
      rv = true;
      }
  configData.endArray();
  configData.endGroup();

  return rv;
  }


std::vector<Operation*> Kernel::loadOperations(ProjectFile* pf) {
  std::vector<Operation*> rv;
  if (!pf) return rv;

  pf->beginGroup("Work");
  int        mxi = pf->beginReadArray("Operations");
  Operation* op;

  for (int i=0; i < mxi; ++i) {
      pf->setArrayIndex(i);

      op = new Operation();
      op->restore(pf->settings());
      rv.push_back(op);
      }
  pf->endArray();
  pf->endGroup();

  return rv;
  }


bool Kernel::loadProject(const QString &fileName) {
  QString modelFile;

  pf = new ProjectFile(fileName);
  pf->beginGroup("Setup");
  modelFile = pf->value("Model-File").toString();
  if (modelFile.endsWith(".brep")) topShape = helper->loadBRep(modelFile);
  else                             topShape = helper->loadStep(modelFile);
  pf->endGroup();
  win.setWindowTitle(QString("- %1 -- %2 -").arg(app.applicationName()).arg(modelFile));
  setupPage->loadProject(pf, topShape);
  operations->loadProject(pf);

  return true;
  }


void Kernel::loadVise(ViseEntry* vise, Handle(AIS_Shape)& left, Handle(AIS_Shape)& middle, Handle(AIS_Shape)& right) {
  left.Nullify();
  middle.Nullify();
  right.Nullify();

  if (!vise->left().isEmpty()) {
     if (vise->left().endsWith(".brep")) {
        left = new AIS_Shape(helper->loadBRep(vise->left()));
        }
     else if (vise->left().endsWith(".step")
           || vise->left().endsWith(".stp")) {
        left = new AIS_Shape(helper->loadStep(vise->left()));
        }
     }
  if (!vise->middle().isEmpty()) {
     if (vise->middle().endsWith(".brep")) {
        middle = new AIS_Shape(helper->loadBRep(vise->middle()));
        }
     else if (vise->middle().endsWith(".step")
           || vise->middle().endsWith(".stp")) {
        middle = new AIS_Shape(helper->loadStep(vise->middle()));
        }
     }
  if (!vise->right().isEmpty()) {
     if (vise->right().endsWith(".brep")) {
        right = new AIS_Shape(helper->loadBRep(vise->right()));
        }
     else if (vise->right().endsWith(".step")
           || vise->right().endsWith(".stp")) {
        right = new AIS_Shape(helper->loadStep(vise->right()));
        }
     }
  }


bool Kernel::loadViseList() {
  configData.beginGroup("Work");
  int        size = configData.beginReadArray("Vises");
  bool       rv   = false;
  ViseEntry* ve;

  for (int i=0; i < size; ++i) {
      ve = new ViseEntry();

      configData.setArrayIndex(i);
      ve->setName(configData.value("name").toString());
      ve->setLeft(configData.value("left").toString());
      ve->setMiddle(configData.value("middle").toString());
      ve->setRight(configData.value("right").toString());
      viseListModel->add(ve);
      rv = true;
      }
  configData.endArray();
  configData.endGroup();

  return rv;
  }


void Kernel::onShutdown(QCloseEvent* ce) {
  QMap<QString, ApplicationWindow*>::const_iterator i = pages.constBegin();

  while (i != pages.constEnd()) {
        qDebug() << "shutdown page:" << i.key();
        i.value()->closeEvent(ce);
        ++i;
        }
  configData.beginGroup("MainWindow");
  configData.setValue("geometry",    win.saveGeometry());
  configData.setValue("windowState", win.saveState());
  configData.setValue("spGeom", win.sp->saveGeometry());
  configData.setValue("spState", win.sp->saveState());
  configData.endGroup();
  }
