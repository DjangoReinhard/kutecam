/* 
 * **************************************************************************
 * 
 *  file:       core.cpp
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
#include "core.h"
#include "applicationwindow.h"
#include "kernel.h"
#include "geomnodemodel.h"
#include "kuteCAM.h"
#include "shapelistmodel.h"
#include "postprocessor.h"
#include "pluginlistmodel.h"
#include "projectfile.h"
#include "occtviewer.h"
#include "operationspage.h"
#include "mainwindow.h"
#include "toollistmodel.h"
#include "work.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPluginLoader>
#include <QDir>
#include <QDebug>


Core::Core(QApplication& app, MainWindow& win)
 : QObject(nullptr)  {
  if (k) throw std::logic_error("invalid call sequence! Already initialized");
  k = new Kernel(app, win);
  k->initialize();
  }


Core::Core() {
  if (!k) throw std::logic_error("invalid call sequence! Need to initialize before use!");
  }


void Core::addCurve(Handle(AIS_Shape) s) {
  k->shapeListModel->append(s);
  k->win.update();
  }


QString Core::appName() const {
  return k->app.applicationName();
  }


QApplication& Core::application() const {
  return k->app;
  }


QSettings& Core::cfg() {
  return k->configData;
  }


const QSettings& Core::cfg() const {
  return k->configData;
  }


QString Core::chooseCADFile(QWidget* parent) {
  QString     selectedFilter, fileName;
  QFileDialog dialog(parent
                   , tr("QFileDialog::getOpenFileName()")
                   , kute::CADBase
                   , kute::CADFilter);

  dialog.setSupportedSchemes(QStringList(QStringLiteral("file")));
  dialog.setMinimumWidth(1200);
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setOption(QFileDialog::DontUseNativeDialog);

  if (dialog.exec() != QDialog::Accepted) return QString();
  selectedFilter = dialog.selectedNameFilter();
  fileName       = dialog.selectedUrls().value(0).toLocalFile();

  qDebug() << "choosen CAD file is:" << fileName;

  return fileName;
  }


QString Core::chooseProjectFile(QWidget* parent) {
  QString     fileName;
  QFileDialog dialog(parent
                   , tr("QFileDialog::getSaveFileName()")
                   , kute::BasePath
                   , tr("Project Files (*.prj)"));

  dialog.setSupportedSchemes(QStringList(QStringLiteral("file")));
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted) {
     qDebug() << "Oups - file-dialog aborted!";
     return fileName;
     }
  fileName = dialog.selectedUrls().value(0).toLocalFile();
  if (!fileName.endsWith(".prj")) fileName += ".prj";

  qDebug() << "save project file to:" << fileName;

  return fileName;
  }


void Core::clearCurves() {
  k->shapeListModel->clear();
  }


bool Core::isAllInOneOperation() const {
  return k->opAllInOne;
  }


bool Core::isAAxisTable() const {
  return k->AisTable;
  }


bool Core::isBAxisTable() const {
  return k->BisTable;
  }


bool Core::isCAxisTable() const {
  return k->CisTable;
  }


bool Core::isSepWithToolChange() const {
  return k->genSepWithToolChange;
  }

bool Core::hasModelLoaded() const {
  return k->pf != nullptr;
  }


Util3D* Core::helper3D() {
  return k->helper;
  }

bool Core::loadFile(const QString &fileName) {
  return k->loadModelFile(fileName);
  }


std::vector<Operation*> Core::loadOperations(ProjectFile *pf) {
  return k->loadOperations(pf);
  }


PostProcessor* Core::loadPostProcessor(const QString& ppName) {
  QString ppPath = k->ppModel->value(ppName);
  QPluginLoader loader(ppPath);
  QObject*      plugin = loader.instance();

  if (plugin) return qobject_cast<PostProcessor*>(plugin);
  return nullptr;
  }


bool Core::loadProject(const QString &fileName) {
  return k->loadProject(fileName);
  }


bool Core::loadTools(const QString &fileName) {
  int os = k->toolListModel->rowCount();

  k->loadTools(fileName);

  return k->toolListModel->rowCount() > os;
  }


void Core::loadVise(ViseEntry* vise, Handle(AIS_Shape)& left, Handle(AIS_Shape)& middle, Handle(AIS_Shape)& right) {
  return k->loadVise(vise, left, middle, right);
  }


MainWindow* Core::mainWin() {
  return &k->win;
  }


bool Core::move2Backup(const QString& fileName) {
  QString   backupPat(fileName);
  QFileInfo fi(fileName);
  QString   extension(QString(".%1").arg(fi.completeSuffix()));

  backupPat.replace(extension, ".bak%1");
  QFileInfo check(backupPat.arg(""));

  if (check.exists()) {
     QFile last(backupPat.arg(9));

     if (last.exists()) last.remove();
     for (int i=8; i > 0; --i) {
         QFile tmp(backupPat.arg(i));

         if (tmp.exists()) tmp.rename(backupPat.arg(i+1));
         }
     QFile tmp(check.absoluteFilePath());

     tmp.rename(backupPat.arg(1));
     }
  QFile file(fileName);

  return file.rename(check.absoluteFilePath());
  }


void Core::onShutdown(QCloseEvent* ce) {
  k->onShutdown(ce);
  }


QString Core::postProcessor() const {
  return k->selectedPP;
  }


QAbstractItemModel* Core::ppModel() const {
  return k->ppModel;
  }


ProjectFile* Core::projectFile() {
  return k->pf;
  }


void Core::riseError(const QString &msg) {
  QMessageBox::critical(nullptr, tr("System Error"), msg);
  }


void Core::saveOperations() {
  k->operations->saveOperations();
  }


SelectionHandler* Core::selectionHandler() {
  return k->selHdr;
  }


void Core::setAllInOneOperation(bool value) {
  k->opAllInOne = value;
  }


void Core::setAAxisIsTable(bool value) {
  k->AisTable = value;
  }


void Core::setBAxisIsTable(bool value) {
  k->BisTable = value;
  }


void Core::setCAxisIsTable(bool value) {
  k->CisTable = value;
  }


void Core::setPostProcessor(const QString& ppName) {
  k->selectedPP = ppName;
  }


void Core::setProjectFile(ProjectFile *pf) {
  k->pf = pf;
  }


void Core::setSepWithToolChange(bool value) {
  k->genSepWithToolChange = value;
  }


void Core::setWorkData(Work *data) {
  k->work = data;
  }


ShapeFix_ShapeTolerance& Core::shapeFix() {
  return k->shapeTolerance;
  }


void Core::switchPage(const QString &page) {
  qDebug() << "requested page:" << page;
  if (k->pages.contains(page)) {
     qDebug() << "OK, switch to" << page;
     k->win.setPage(k->pages[page]);
     }
  }


TDFactory* Core::tdFactory() {
  return k->tdFactory;
  }


ToolListModel* Core::toolListModel() {
  return k->toolListModel;
  }


Ui::MainWindow* Core::uiMainWin() {
  return k->win.ui;
  }


OcctQtViewer* Core::view3D() {
  return k->view3D;
  }


Work* Core::workData() {
  return k->work;
  }


WSFactory* Core::wsFactory() {
  return k->wsFactory;
  }


Kernel* Core::k = nullptr;
const QString Core::PgWorkPiece  = tr("Workpiece");
const QString Core::PgConfig     = tr("Preferences");
const QString Core::PgOperations = tr("Operations");
