/* 
 * **************************************************************************
 * 
 *  file:       kernel.h
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
#ifndef KERNEL_H
#define KERNEL_H
#include <QObject>
#include <QDir>
#include <QMap>
#include <QSettings>
#include <AIS_Shape.hxx>
#include <TopoDS_Shape.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
class ApplicationWindow;
class OcctQtViewer;
class MainWindow;
class ProjectFile;
class Util3D;
class ConfigPage;
class StringListModel;
class Operation;
class OperationsPage;
class PluginListModel;
class SelectionHandler;
class SetupPage;
class ShapeListModel;
class TDFactory;
class ToolListModel;
class ViseEntry;
class ViseListModel;
class Work;
class WSFactory;
class QAbstractItemModel;
class QApplication;
class QCloseEvent;
class QFileInfo;


class Kernel : public QObject
{
  Q_OBJECT
public:
  explicit Kernel(QApplication& app, MainWindow& win);

  bool loadConfig();
  bool loadModelFile(const QString& fileName);
  bool loadProject(const QString& fileName);
  std::vector<Operation*> loadOperations(ProjectFile* pf);

protected:
  void getPostProcessors();
  std::vector<QFileInfo*> findFile(const QDir& dir, const QStringList& nameFilters, QDir::Filters defFilters);
  void initialize();  
  bool loadMaterials();
  bool loadTools(const QString &fileName);
  bool loadViseList();
  void loadVise(ViseEntry* vise, Handle(AIS_Shape)& left, Handle(AIS_Shape)& middle, Handle(AIS_Shape)& right);
  void onShutdown(QCloseEvent* ce);
  void processAppArgs(const QStringList& args);
  QLocale* setupTranslators();

protected slots:
  void clearCurves();

private:
  QApplication&                     app;
  MainWindow&                       win;
  QLocale*                          curLocale;
  QSettings                         configData;
  QMap<QString, ApplicationWindow*> pages;
  ShapeFix_ShapeTolerance           shapeTolerance;
  OcctQtViewer*                     view3D;
  Util3D*                           helper;
  SelectionHandler*                 selHdr;
  ProjectFile*                      pf;
  TopoDS_Shape                      topShape;
  Work*                             work;
  OperationsPage*                   operations;
  ConfigPage*                       config;
  int                               machineType;
  bool                              autoRotate;
  bool                              AisTable;
  bool                              BisTable;
  bool                              CisTable;  
  bool                              genSepWithToolChange;
  bool                              opAllInOne;
  QString                           langDir;
  QString                           selectedPP;
  SetupPage*                        setupPage;
  TDFactory*                        tdFactory;
  WSFactory*                        wsFactory;
  StringListModel*                  matModel;
  ShapeListModel*                   shapeListModel;
  ViseListModel*                    viseListModel;
  ToolListModel*                    toolListModel;
  PluginListModel*                  ppModel;
  friend class Core;  
  };
#endif // KERNEL_H
