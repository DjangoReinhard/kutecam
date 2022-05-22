/* 
 * **************************************************************************
 * 
 *  file:       preview3d.cpp
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
#include "preview3d.h"
#include <ui_preview.h>
#include "ui_mainwindow.h"
#include "core.h"
#include "editorpage.h"
#include "occtviewer.h"
#include "work.h"
#include <QLabel>


Preview3D::Preview3D(QWidget *parent)
 : QWidget(parent)
 , ui(new Ui::Preview3D)
 , view3D(new OcctQtViewer())
 , edit(new EditorPage()) {
  ui->setupUi(this);
  ui->notebook->addTab(view3D, tr("Preview"));
  ui->notebook->addTab(edit, tr("Editor"));

  connect(ui->pbBottom,    &QPushButton::clicked, view3D, &OcctQtViewer::bottomView);
  connect(ui->pbLeft,      &QPushButton::clicked, view3D, &OcctQtViewer::leftView);
  connect(ui->pbRight,     &QPushButton::clicked, view3D, &OcctQtViewer::rightView);
  connect(ui->pbFront,     &QPushButton::clicked, view3D, &OcctQtViewer::frontView);
  connect(ui->pbBack,      &QPushButton::clicked, view3D, &OcctQtViewer::backView);
  connect(ui->pbIso1,      &QPushButton::clicked, view3D, &OcctQtViewer::iso1View);
  connect(ui->pbIso2,      &QPushButton::clicked, view3D, &OcctQtViewer::iso2View);
  connect(ui->pbIso3,      &QPushButton::clicked, view3D, &OcctQtViewer::iso3View);
  connect(ui->pbIso4,      &QPushButton::clicked, view3D, &OcctQtViewer::iso4View);
  connect(ui->pbTop,       &QPushButton::clicked, view3D, &OcctQtViewer::topView);
  connect(ui->pbFit,       &QPushButton::clicked, view3D, &OcctQtViewer::fitAll);
  connect(ui->pbWireframe, &QPushButton::clicked, this,   &Preview3D::toggleWireframe);
  connect(ui->pbHide,      &QPushButton::clicked, this,   &Preview3D::toggleHide);

  connect(Core().uiMainWin()->actionOrthografic, &QAction::triggered, view3D, &OcctQtViewer::switchOrthographic);
  connect(Core().uiMainWin()->actionTop,         &QAction::triggered, view3D, &OcctQtViewer::topView);
  connect(Core().uiMainWin()->actionBottom,      &QAction::triggered, view3D, &OcctQtViewer::bottomView);
  connect(Core().uiMainWin()->actionFront,       &QAction::triggered, view3D, &OcctQtViewer::frontView);
  connect(Core().uiMainWin()->actionBack,        &QAction::triggered, view3D, &OcctQtViewer::backView);
  connect(Core().uiMainWin()->actionLeft,        &QAction::triggered, view3D, &OcctQtViewer::leftView);
  connect(Core().uiMainWin()->actionRight,       &QAction::triggered, view3D, &OcctQtViewer::rightView);
  connect(Core().uiMainWin()->actionIso1,        &QAction::triggered, view3D, &OcctQtViewer::iso1View);
  connect(Core().uiMainWin()->actionIso2,        &QAction::triggered, view3D, &OcctQtViewer::iso2View);
  connect(Core().uiMainWin()->actionIso3,        &QAction::triggered, view3D, &OcctQtViewer::iso3View);
  connect(Core().uiMainWin()->actionIso4,        &QAction::triggered, view3D, &OcctQtViewer::iso4View);
  connect(Core().uiMainWin()->actionZoom2Fit,    &QAction::triggered, view3D, &OcctQtViewer::fitAll);
  connect(Core().uiMainWin()->actionWireframe,   &QAction::triggered, view3D, &OcctQtViewer::switchWireframe);
  }


Preview3D::~Preview3D() {
  delete ui;
  delete view3D;
  }


void Preview3D::loadFile(const QString& fileName) {
  edit->loadFile(fileName);
  ui->notebook->setCurrentWidget(edit);
  }


void Preview3D::toggleHide() {
  OcctQtViewer* v3D = Core().view3D();

  Core().uiMainWin()->actionHideModel->toggle();
  if (Core().uiMainWin()->actionHideModel->isChecked()) {
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


void Preview3D::toggleWireframe() {
  Core().uiMainWin()->actionWireframe->toggle();
  Core().view3D()->switchWireframe(Core().uiMainWin()->actionWireframe->isChecked());
  }
