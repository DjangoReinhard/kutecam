/* 
 * **************************************************************************
 * 
 *  file:       main.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    8.4.2022 by Django Reinhard
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
#include "core.h"
#include <QApplication>
#include <QDebug>


int main(int argc, char *argv[]) {
  int rv = -1;

  try {
      QApplication a(argc, argv); a.setApplicationName("kuteCAM V0.01");
      MainWindow   w;
      Core         core(a, w);

      qDebug() << "application name:" << a.applicationName();

      w.show();
      rv = a.exec();
      }
  catch (const QString& s) {
      qDebug() << s;
      }
  catch (const std::exception& e) {
      qDebug() << e.what();
      }
  return rv;
  }
