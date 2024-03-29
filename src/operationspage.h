/* 
 * **************************************************************************
 * 
 *  file:       operationspage.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    23.4.2022 by Django Reinhard
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
#ifndef OPERATIONSPAGE_H
#define OPERATIONSPAGE_H
#include "applicationwindow.h"
#include "operation.h"
#include <TopoDS_Shape.hxx>
#include <QBasicTimer>
#include <QMap>
QT_BEGIN_NAMESPACE
namespace Ui {
class OperationsPage;
}
QT_END_NAMESPACE
class GeomNodeModel;
class OperationListModel;
class OperationSubPage;
class PathBuilder;
class ProjectFile;
class Work;
class QItemSelection;
class QStackedLayout;
class QStringListModel;
class TargetDefListModel;
class ToolEntry;


class OperationsPage : public ApplicationWindow
{
  Q_OBJECT
public:
  explicit OperationsPage(QWidget *parent = nullptr);

  void calcRotation4(const gp_Dir &n, double& aA, double& aB, double& aC);
  void clear();
  void closeEvent(QCloseEvent* e) override;
  void createTool(int toolNum);
  bool eventFilter(QObject *obj, QEvent *event) override;
  void loadOperation(Operation* op);
  void loadProject(ProjectFile* pf);
  void saveOperations();
  void opSelected(const QItemSelection& selected, const QItemSelection& deselected);
  void rotate();
  void rotateIfFace(const std::vector<TopoDS_Shape>& selection);
  void shapeSelected(const TopoDS_Shape& shape);

public slots:
  void addOperation(Operation* op);
  void cutDepthChanged(double v);
  void selectionChanged();
  void genGCode();
  void handleMachineType(int machineType);
  void reSelect();
  void sel2Horizontal();
  void sel2Vertical();
  void simulate();
  void toggleToolpath();
  void toolPath();

signals:
  void fileGenerated(const QString& fileName);
  void raiseMessage(const QString& msg);
  void modelChanged(const Bnd_Box& bb);

private:
  Ui::OperationsPage*              ui;
  OperationListModel*              olm;
  Operation*                       currentOperation;
  QStackedLayout*                  opStack;
  GeomNodeModel*                   infoModel;
  PathBuilder*                     pathBuilder;
  QMap<QString, OperationSubPage*> pages;
  OperationSubPage*                subPage;
  std::vector<TargetDefinition*>   dummy;
  TargetDefListModel*              tdModel;
  };
#endif // OPERATIONSPAGE_H
