/* 
 * **************************************************************************
 * 
 *  file:       core.cpp
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
#include "core.h"
#include "kernel.h"
#include "geomlistmodel.h"
#include "shapelistmodel.h"
#include "projectfile.h"
#include "occtviewer.h"
#include "mainwindow.h"
#include "toolpage.h"
#include "work.h"
#include <QApplication>
#include <QFileDialog>
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
                   , "/media/CAD"
                   , tr("BRep Files (*.brep);;Step Files (*.stp *.step)"));

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


void Core::clearCurves() {
//  k->geoListModel->clear();
  k->shapeListModel->clear();
  }


bool Core::isAaxisTable() const {
  return k->AisTable;
  }


bool Core::isBaxisTable() const {
  return k->BisTable;
  }


bool Core::isCaxisTable() const {
  return k->CisTable;
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


bool Core::loadProject(const QString &fileName) {
  return k->loadProject(fileName);
  }


bool Core::loadTools(const QString &fileName) {
  return k->tools->readTools(fileName);
  }


void Core::loadVise(ViseEntry* vise, Handle(AIS_Shape)& left, Handle(AIS_Shape)& middle, Handle(AIS_Shape)& right) {
  return k->loadVise(vise, left, middle, right);
  }


MainWindow* Core::mainWin() {
  return &k->win;
  }


void Core::onShutdown(QCloseEvent* ce) {
  k->onShutdown(ce);
  }


ProjectFile* Core::projectFile() {
  return k->pf;
  }


SelectionHandler* Core::selectionHandler() {
  return k->selHdr;
  }


void Core::setProjectFile(ProjectFile *pf) {
  k->pf = pf;
  }


void Core::setWorkData(Work *data) {
  k->work = data;
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
const QString Core::PgWorkPiece    = tr("Workpiece");
const QString Core::PgInfo         = tr("Info");
const QString Core::PgConfig       = tr("Config");
const QString Core::PgOperations   = tr("Operations");
const QString Core::PgTools        = tr("Tools");
