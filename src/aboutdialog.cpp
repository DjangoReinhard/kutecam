/* 
 * **************************************************************************
 * 
 *  file:       aboutdialog.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements
 *  created:    22.1.2022 by Django Reinhard
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
#include "aboutdialog.h"
#include "core.h"
#include "occtviewer.h"
#include <QHeaderView>
#include <QTabWidget>
#include <QListView>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <Standard_Version.hxx>
#include <qconfig.h>


AboutDialog::AboutDialog(QWidget* parent)
 : QDialog(parent) {
  tw = new QTabWidget;

  tw->addTab(new GeneralTab(), tr("General"));
  tw->addTab(new GLInfoTab(),  tr("GL-Info"));
  tw->addTab(new PluginsTab(), tr("Plugins"));

  bb = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
  QLabel*      l  = new QLabel(tr("<h3>kuteCAM</h3>"));
  QVBoxLayout* vl = new QVBoxLayout;

  vl->addWidget(l);
  vl->addWidget(tw);
  vl->addWidget(bb);
  setLayout(vl);
  setWindowTitle("About - rednose");
  resize(400,400);
  }


GeneralTab::GeneralTab(QWidget* parent)
 : QWidget(parent) {
  QLabel* l = new QLabel(tr("<h2>kuteCAM</h2><p>is an application to create "
                            "toolpaths from CAD model.</p>"
                            "<p>Load a CAD model (actually *.brep or *.step), "
                            "select the part(s) you want to mill and let <b>kuteCAM</b> "
                            "generate the toolpath and write the code for your machine.</p>"
                            "<p>Code generation is performed by postprocessors, which are "
                            "Qt-plugins loaded at runtime. See \"Plugins\"-tab for list of "
                            "available postprocessors.</p>"
                            "<p><b>kuteCAM</b> was built with these external components:<ul>"
                            "<li><a href=\"%1\">Open CASCADE Technology v.%2</a></li>"
                            "<li><a href=\"%3\">Qt Toolkit v.%4</a></li></ul></p>"
                            "<p></p>"
                            "<h3>More Info at</h3><p><ul>"
                            "<li><a href=\"%5\">kuteCAM Wiki</a></li>"
                            "</ul></p>"
                            )
                         .arg("https://dev.opencascade.org/release",  OCC_VERSION_STRING_EXT)
                         .arg("https://www.qt.io/download", QLatin1String(QT_VERSION_STR))
                         .arg("https://github.com/DjangoReinhard/kutecam/wiki")
                         );
  QVBoxLayout* vl = new QVBoxLayout;

  l->setWordWrap(true);
  vl->addWidget(l);
  setLayout(vl);
  }


GLInfoTab::GLInfoTab(QWidget* parent)
 : QWidget(parent) {
  QString glInfo = Core().view3D()->getGlInfo();
  QLabel* l      = new QLabel(tr("GL capabilities: %1")
                             .arg(glInfo)
                              );
  QVBoxLayout* vl = new QVBoxLayout;

  vl->addWidget(l);
  setLayout(vl);
  }


PluginsTab::PluginsTab(QWidget* parent)
 : QWidget(parent)
 , list(new QListView) {
  list->setAlternatingRowColors(true);
  list->setSelectionMode(QAbstractItemView::NoSelection);
  list->setModel(Core().ppModel());
  QVBoxLayout* vl = new QVBoxLayout;

  vl->addWidget(list);
  setLayout(vl);
  }

