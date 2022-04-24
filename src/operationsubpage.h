/* 
 * **************************************************************************
 * 
 *  file:       operationsubpage.h
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
#ifndef OPERATIONSUBPAGE_H
#define OPERATIONSUBPAGE_H
#include <QWidget>
#include "operation.h"
#include <gp_Dir.hxx>
QT_BEGIN_NAMESPACE
namespace Ui {
class OpSub;
}
QT_END_NAMESPACE
class PathBuilder;
class ToolEntry;
class QStringListModel;
class OperationListModel;
class TargetDefListModel;


class OperationSubPage : public QWidget
{
  Q_OBJECT
public:
  explicit OperationSubPage(OperationListModel* olm, TargetDefListModel* tdModel, QWidget *parent = nullptr);
  virtual ~OperationSubPage() = default;

  virtual void loadOP(Operation* op);
  virtual void processSelection() = 0;
  virtual void showToolPath() = 0;
  virtual void toolPath() = 0;
  virtual void closeEvent(QCloseEvent* e) override;

public slots:
  void absToggled(const QVariant& v);
  void coolingChanged(const QVariant& v);
  void cutDepthChanged(double v);
  void cutFeedChanged(double v);
  void cutSpeedChanged(double v);
  void cutWidthChanged(double v);
  void cycleChanged(const QVariant& v);
  void dirChanged(const QVariant& v);
  void finalDepthChanged(double v);
  void fixit();
  void fixtureChanged(int i);
  void offsetChanged(double v);
  void opNameChanged(const QString& name);
  void outToggled(const QVariant& v);
  void r1Changed(double v);
  void r2Changed(double v);
  void toolChanged(const QVariant& i);
  void typeChanged(const QVariant& v);

protected:
  virtual void connectSignals();
  Operation*   createOP(int id, const QString& name, OperationType type);
  QStringList  genCycleList();
  virtual void processTargets();

signals:
  void opCreated(Operation* op);
  void modelChanged(const Bnd_Box& bb);

protected:
  Ui::OpSub*          ui;
  OperationListModel* olm;
  Operation*          curOP;
  ToolEntry*          activeTool;
  PathBuilder*        pathBuilder;
  QStringListModel*   opTypes;
  QStringListModel*   dirModel;
  QStringListModel*   drillCycles;
  QStringListModel*   fixModel;
  QStringListModel*   coolingModes;
  TargetDefListModel* tdModel;
  };
#endif // OPERATIONSUBPAGE_H
