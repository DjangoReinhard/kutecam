/* 
 * **************************************************************************
 * 
 *  file:       core.h
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
#ifndef CORE_H
#define CORE_H
#include <QObject>
#include <AIS_Shape.hxx>
#include <Geom_Curve.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE
class Kernel;
class MainWindow;
class OcctQtViewer;
class Operation;
class PostProcessor;
class ProjectFile;
class SelectionHandler;
class TDFactory;
class ToolListModel;
class Util3D;
class ViseEntry;
class Work;
class WSFactory;
class QAbstractItemModel;
class QApplication;
class QCloseEvent;
class QSettings;


class Core : public QObject
{
  Q_OBJECT
public:
  static const QString PgWorkPiece;
  static const QString PgConfig;
  static const QString PgOperations;

  explicit Core();
  explicit Core(QApplication& app, MainWindow& win);
  virtual ~Core() = default;

  void                     addCurve(Handle(AIS_Shape) s);
  QString                  appName() const;
  Ui::MainWindow*          uiMainWin();
  MainWindow*              mainWin();
  QSettings&               cfg();
  const QSettings&         cfg() const;
  QString                  chooseCADFile(QWidget* parent = nullptr);
  void                     clearCurves();
  Util3D*                  helper3D();
  bool                     hasModelLoaded() const;
  bool                     isAllInOneOperation() const;
  bool                     isAAxisTable() const;
  bool                     isBAxisTable() const;
  bool                     isCAxisTable() const;
  bool                     isSepWithToolChange() const;
  bool                     loadFile(const QString& fileName);
  std::vector<Operation*>  loadOperations(ProjectFile* pf);
  PostProcessor*           loadPostProcessor(const QString& ppName);
  bool                     loadProject(const QString& fileName);
  bool                     loadTools(const QString& fileName);
  void                     loadVise(ViseEntry* vise, Handle(AIS_Shape)& left, Handle(AIS_Shape)& middle, Handle(AIS_Shape)& right);
  bool                     move2Backup(const QString& fileName);
  void                     onShutdown(QCloseEvent* ce);
  QString                  postProcessor() const;
  QAbstractItemModel*      ppModel() const;
  ProjectFile*             projectFile();
  void                     riseError(const QString& msg);
  ShapeFix_ShapeTolerance& shapeFix();
  SelectionHandler*        selectionHandler();
  void                     setAllInOneOperation(bool value);
  void                     setAAxisIsTable(bool value);
  void                     setBAxisIsTable(bool value);
  void                     setCAxisIsTable(bool value);
  void                     setPostProcessor(const QString& ppName);
  void                     setProjectFile(ProjectFile* pf);
  void                     setSepWithToolChange(bool value);
  void                     setWorkData(Work* data);
  void                     switchPage(const QString& page);
  TDFactory*               tdFactory();
  ToolListModel*           toolListModel();
  OcctQtViewer*            view3D();
  Work*                    workData();
  WSFactory*               wsFactory();

private:
  static Kernel* k;
  };
#endif // CORE_H
