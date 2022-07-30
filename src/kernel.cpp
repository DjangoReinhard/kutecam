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
#include "editorpage.h"
#include "projectfile.h"
#include "postprocessor.h"
#include "work.h"
#include "mainwindow.h"
#include "configpage.h"
#include "setuppage.h"
#include "util3d.h"
#include "occtviewer.h"
#include "geomnodemodel.h"
#include "setuppage.h"
#include "operationspage.h"
#include "pluginlistmodel.h"
#include "preview3d.h"
#include "selectionhandler.h"
#include "shapelistmodel.h"
#include "tdfactory.h"
#include "toollistmodel.h"
#include "viseentry.h"
#include "viselistmodel.h"
#include "wsfactory.h"
#include "xmltoolreader.h"
#include <BRepLib.hxx>
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QPluginLoader>
#include <QSplitter>
#include <QTranslator>


Kernel::Kernel(QApplication& app, MainWindow& win)
 : QObject(nullptr)
 , app(app)
 , win(win)
 , curLocale(nullptr)
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
 , shapeListModel(new ShapeListModel(this))
 , viseListModel(new ViseListModel(this))
 , toolListModel(new ToolListModel(this))
 , ppModel(new PluginListModel(this)) {
  BRepLib::Precision(1e-4);
  }


void Kernel::clearCurves() {
  shapeListModel->clear();
  }


std::vector<QFileInfo*> Kernel::findFile(const QDir& dir, const QStringList& nameFilters, QDir::Filters defFilter) {
  QDir curDir(dir);
  std::vector<QFileInfo*> rv;

//  qDebug() << "check path:" << curDir.absolutePath();

  curDir.setFilter(defFilter);
  curDir.setNameFilters(nameFilters);
  QFileInfoList list = curDir.entryInfoList();

  for (int i = 0; i < list.size(); ++i) {
      QFileInfo fileInfo = list.at(i);

      if (fileInfo.isDir()) {
//         qDebug() << "directory" << fileInfo.absoluteFilePath();
         std::vector<QFileInfo*> rv0 = findFile(QDir(fileInfo.absoluteFilePath()), nameFilters, defFilter);

         if (rv0.size()) {
            for (auto& fi : rv0) rv.push_back(fi);
            }
         }
      else {
//         std::cout << qPrintable(QString("%1 %2").arg(fileInfo.size(), 10)
//                                                 .arg(fileInfo.fileName()));
//         std::cout << std::endl;
         rv.push_back(new QFileInfo(fileInfo));
         }
      }
  return rv;
  }


void Kernel::getPostProcessors() {
  QStringList   sl;
  QDir          ppBase("./pp");
  QDir::Filters defFilter = QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot;

  sl << "lib*.so";

  std::vector<QFileInfo*> res = findFile(ppBase, sl, defFilter);

  // validate plugins by loading them. But don't waste mem for unused postprocessors,
  // so unload them, until we need one of them for a real job
  for (auto& fi : res) {
      qDebug() << "found possible plugin: " << fi->absoluteFilePath();

      QPluginLoader loader(fi->absoluteFilePath());
      QObject*      plugin = loader.instance();
      QString       name = fi->fileName().mid(5, fi->fileName().size() - 8);

      if (plugin) {
         auto iPlugin = qobject_cast<PostProcessor*>(plugin);

         qDebug() << "library " << name << "seems to be valid postprocessor";
         ppModel->setData(name, fi->absoluteFilePath());
         if (!loader.unload()) {
            qDebug() << "failed to unload plugin" << name << "!!!";
            }
         }
      else {
         qDebug() << "plugin-loader error string: " << loader.errorString();
         }
      }
  }


void Kernel::initialize() {
  QDir dir(QCoreApplication::applicationDirPath());

  dir.cd("i18n");
  langDir = dir.absolutePath();
  processAppArgs(app.arguments());
  curLocale = setupTranslators();

  win.initialize();
  getPostProcessors();
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
  connect(operations, &OperationsPage::fileGenerated, win.editor, &EditorPage::loadFile);
  connect(config, &ConfigPage::machineTypeChanged, operations, &OperationsPage::handleMachineType);
  configData.setValue("what", "nope");
  }


bool Kernel::loadConfig() {
  bool rv = loadMaterials();

  configData.beginGroup("Work");
  selectedPP = configData.value("PP").toString();
  AisTable = configData.value("A-is-table").toBool();
  BisTable = configData.value("B-is-table").toBool();
  CisTable = configData.value("C-is-table").toBool();
  opAllInOne = configData.value("opAllInOne").toBool();
  autoRotate = configData.value("autoRotateSelected").toBool();
  machineType = configData.value("machineType").toInt();
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
  pf = new ProjectFile(fileName);
  pf->beginGroup("Setup");
  QString modelFile = pf->value("Model-File").toString();
  QFile   tfn       = pf->value("Tool-file").toString();

  if (modelFile.endsWith(".brep")) topShape = helper->loadBRep(modelFile);
  else                             topShape = helper->loadStep(modelFile);
  pf->endGroup();
  if (tfn.exists()) loadTools(tfn.fileName());
  win.setWindowTitle(QString("- %1 -- %2 -").arg(app.applicationName(), fileName));
  setupPage->loadProject(pf, topShape);
  operations->loadProject(pf);

  return true;
  }


bool Kernel::loadTools(const QString &fileName) {
  QFile inFile(fileName);
  XmlToolReader xtr;

  if (inFile.exists()) {
     toolListModel->setData(xtr.read(&inFile));
     inFile.close();

     return true;
     }
  return false;
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
  if (!pf) return;
  const QString& tfn = pf->tempFileName();
  const QString& fn  = pf->fileName();

  if (tfn == fn) {
     qDebug() << "possibly have to delete temporary project file ...";
     QFile::remove(tfn);
     }
  }


void Kernel::processAppArgs(const QStringList& args) {
  int mx = args.size();

  for (int i=0; i < mx; ++i) {
      if (args[i]      == "-i18n" && mx > (i+1)) langDir = args[++i];
      else if (args[i] == "-noi18n") langDir = "/tmp";
      }
  }


QLocale* Kernel::setupTranslators() {
  QLocale           sysLocale;
  QLocale::Language lang    = sysLocale.language();
  QLocale::Country  country = sysLocale.country();
  QLocale*          curLocale = new QLocale(lang, country);
  QDir              i18nDir(langDir);
  const auto        entryList = i18nDir.entryList(QDir::Files);
  bool              ok;

  qDebug() << "language:" << lang;
  qDebug() << "country:" << country;
  qDebug() << "syslocale:" << sysLocale.name() << "\tcurrent locale:" << curLocale->name();

  for (const QString& s : entryList) {
      if (!s.endsWith(".qm")) continue;
      int          ndx  = s.indexOf('_');
      QString      name = s.mid(0, ndx);
      QTranslator* tr   = new QTranslator();

      ok = tr->load(*curLocale, name, "_", i18nDir.absolutePath());

      qDebug() << "translation-messages:" << name << (ok ? "loaded" : "FAILED to load");

      if (ok) {
         app.installTranslator(tr);
         break;
         }
      else delete tr;
      }
  return curLocale;
  }
