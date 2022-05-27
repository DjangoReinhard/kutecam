/* 
 * **************************************************************************
 * 
 *  file:       mainwindow.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    10.4.2022 by Django Reinhard
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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QString>
#include <Geom_Curve.hxx>
#include <vector>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
class StatusPos;
}
QT_END_NAMESPACE
class Bnd_Box;
class EditorPage;
class OcctQtViewer;
class GeomNodeModel;
class Preview3D;
class Util3D;
class Work;
class QListView;
class QListWidget;
class QSplitter;
class QStackedWidget;
class QTabWidget;


class MainWindow: public QMainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = nullptr);
 ~MainWindow();

  void            initialize();
  Ui::MainWindow* UI();
  void            addPage(QWidget* page);
  void            clearStatus();
  void            closeEvent(QCloseEvent* e) override;
  Preview3D*      preview3D() const;
  void            restore();
  void            setPage(QWidget* page);

public slots:
  void refresh(const Bnd_Box& bb);
  void setStatusMessage(const QString& msg);
  void showModel();

protected:
  void chooseFile();
  void clearCurves();
  void createConnections();
  void openProject();
  void update();
  void saveProject();
  void startTimer();
  OcctQtViewer* viewer3D();

private:
  Ui::MainWindow* ui;
  QStackedWidget* stack;
  QTabWidget*     notebook;
  QSplitter*      sp;
  QTimer*         timer;
  Preview3D*      preview;
  EditorPage*     editor;
  Bnd_Box*        bbModel;
  friend class Core;
  friend class Kernel;
  };
#endif // MAINWINDOW_H
