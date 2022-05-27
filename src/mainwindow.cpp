/* 
 * **************************************************************************
 * 
 *  file:       mainwindow.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    15.4.2022 by Django Reinhard
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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_status.h"
#include "core.h"
#include "editorpage.h"
#include "util3d.h"
#include "occtviewer.h"
#include "preview3d.h"
#include "projectfile.h"
#include "work.h"
#include "kuteCAM.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTimer>
#include <QModelIndex>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
 : QMainWindow(parent)
 , ui(new Ui::MainWindow)
 , stack(new QStackedWidget())
 , timer(nullptr)
 , preview(nullptr)
 , bbModel(nullptr) {
  }


MainWindow::~MainWindow() {
  delete ui;
  }


void MainWindow::addPage(QWidget *page) {
  if (page) stack->addWidget(page);
  }


void MainWindow::clearStatus() {
  ui->xMin->setNum(0);
  ui->yMin->setNum(0);
  ui->zMin->setNum(0);
  ui->xMax->setNum(0);
  ui->yMax->setNum(0);
  ui->zMax->setNum(0);
  ui->material->clear();

  ui->message->setText(tr("load Project or CAD model"));
  }


void MainWindow::closeEvent(QCloseEvent *e) {
  Core().onShutdown(e);
  QMainWindow::closeEvent(e);
  }


void MainWindow::createConnections() {
  connect(ui->actionFileOpen,    &QAction::triggered, this,  &MainWindow::chooseFile);
  connect(ui->actionProjectOpen, &QAction::triggered, this,  &MainWindow::openProject);
  connect(ui->actionProjectSave, &QAction::triggered, this,  &MainWindow::saveProject);
  connect(ui->actionExit,        &QAction::triggered, this,  &QWidget::close);

  connect(ui->actionHideModel,   &QAction::triggered, this,  &MainWindow::showModel);

  connect(ui->actionModelSetup,      &QAction::triggered, this, [=](){ Core().switchPage(Core::PgWorkPiece); });
  connect(ui->actionOperationsSetup, &QAction::triggered, this, [=](){ Core().switchPage(Core::PgOperations); });
  connect(ui->actionPreferences,     &QAction::triggered, this, [=](){ Core().switchPage(Core::PgConfig); });
  }


void MainWindow::chooseFile() {
  QString fileName = Core().chooseCADFile();

  if (fileName.isEmpty()) return;
  Core().loadFile(fileName);
  ui->actionProjectSave->setEnabled(true);
  ui->actionModelSetup->setEnabled(true);
  ui->actionOperationsSetup->setEnabled(false);
  Core().switchPage(Core::PgWorkPiece);
  startTimer();
  }


void MainWindow::initialize() {
  ui->setupUi(this);
  QVBoxLayout* bl = new QVBoxLayout();  

  notebook = new QTabWidget();
  preview  = new Preview3D();
  editor   = new EditorPage();
  notebook->setTabPosition(QTabWidget::South);
  notebook->addTab(preview, tr("Preview"));
  notebook->addTab(editor, tr("Editor"));
  sp = new QSplitter(Qt::Horizontal);
  sp->addWidget(stack);
  sp->addWidget(notebook);
  bl->setContentsMargins(0, 0, 0, 0);
  bl->addWidget(sp);
  ui->main->setLayout(bl);

  createConnections();
  clearStatus();
  }


void MainWindow::openProject() {
  QString     fileName;
  QFileDialog dialog(nullptr
                   , tr("QFileDialog::getOpenFileName()")
                   , kute::BasePath
                   , tr("Project Files (*.prj)"));

  dialog.setSupportedSchemes(QStringList(QStringLiteral("file")));
  dialog.setMinimumWidth(1200);
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setOption(QFileDialog::DontUseNativeDialog);

  if (dialog.exec() != QDialog::Accepted) return;
  fileName = dialog.selectedUrls().value(0).toLocalFile();

  Core().loadProject(fileName);

  ui->actionModelSetup->setEnabled(false);
  ui->actionOperationsSetup->setEnabled(true);
  ui->actionProjectSave->setEnabled(true);
  Core().switchPage(Core::PgOperations);
  ui->message->clear();
  startTimer();
  }


Preview3D* MainWindow::preview3D() const {
  return preview;
  }

void MainWindow::refresh(const Bnd_Box& bb) {
  if (!bbModel) bbModel = new Bnd_Box(bb);
  *bbModel = Bnd_Box(bb.CornerMin(), bb.CornerMax());
  }


void MainWindow::restore() {
  Core().cfg().beginGroup("MainWindow");
  restoreGeometry(Core().cfg().value("geometry").toByteArray());
  restoreState(Core().cfg().value("windowState").toByteArray());
  sp->restoreState(Core().cfg().value("spState").toByteArray());
  sp->restoreGeometry(Core().cfg().value("spGeom").toByteArray());
  Core().cfg().endGroup();
  }


void MainWindow::saveProject() {
  if (!Core().hasModelLoaded()) return;
  qDebug() << "current project file:" << Core().projectFile()->fileName();
  QString     fileName;
  QFileDialog dialog(this
                   , tr("QFileDialog::getSaveFileName()")
                   , kute::BasePath
                   , tr("Project Files (*.prj)"));

  dialog.setSupportedSchemes(QStringList(QStringLiteral("file")));
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted) {
     qDebug() << "Oups - file-dialog aborted!";
     return;
     }
  fileName = dialog.selectedUrls().value(0).toLocalFile();
  if (!fileName.endsWith(".prj")) fileName += ".prj";

  qDebug() << "save project file to:" << fileName;

  QFile tmp(Core().projectFile()->fileName());

  if (tmp.copy(fileName)) Core().setProjectFile(new ProjectFile(fileName));
  else                    qDebug() << "failed to create project file!";
  }


void MainWindow::setPage(QWidget *page) {
  if (page) stack->setCurrentWidget(page);
  }


void MainWindow::setStatusMessage(const QString& msg) {
  ui->message->setText(msg);
  }


void MainWindow::showModel() {
  OcctQtViewer* v3D = Core().view3D();

  if (ui->actionHideModel->isChecked()) {
     std::vector<Handle(AIS_Shape)> shapes = Core().workData()->modShapes;

     for (auto& s : shapes) {
         v3D->removeShape(s);
         }
     }
  else {
     std::vector<Handle(AIS_Shape)> shapes = Core().workData()->modShapes;

     for (auto& s : shapes) {
         v3D->showShape(s);
         }
     }
  v3D->refresh();
  }


void MainWindow::startTimer() {
  if (timer) return;

  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &MainWindow::update);
  timer->start(200);
  }


Ui::MainWindow* MainWindow::UI() {
  return ui;
  }


void MainWindow::update() {
  if (!bbModel) return;
  ui->xMin->setNum(Core().helper3D()->deburr(bbModel->CornerMin().X()));
  ui->yMin->setNum(Core().helper3D()->deburr(bbModel->CornerMin().Y()));
  ui->zMin->setNum(Core().helper3D()->deburr(bbModel->CornerMin().Z()));
  ui->xMax->setNum(Core().helper3D()->deburr(bbModel->CornerMax().X()));
  ui->yMax->setNum(Core().helper3D()->deburr(bbModel->CornerMax().Y()));
  ui->zMax->setNum(Core().helper3D()->deburr(bbModel->CornerMax().Z()));
  ui->material->setText(Core().workData()->material);
  }


OcctQtViewer* MainWindow::viewer3D() {
  return preview ? preview->viewer3D() : nullptr;
  }
