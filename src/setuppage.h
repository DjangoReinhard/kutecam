/* 
 * **************************************************************************
 * 
 *  file:       setuppage.h
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
#ifndef SETUPPAGE_H
#define SETUPPAGE_H
#include "applicationwindow.h"
#include "stringlistmodel.h"
#include <AIS_Shape.hxx>
QT_BEGIN_NAMESPACE
namespace Ui {
class SetupPage;
}
QT_END_NAMESPACE
class ProjectFile;
class QStringList;
class Util3D;
class ViseListModel;
class Work;


class SetupPage : public ApplicationWindow
{
  Q_OBJECT

public:
  explicit SetupPage(StringListModel* matModel, ViseListModel* vises, QWidget *parent = nullptr);

  void changeVise(const QString& vise);
  void createClamping();
  void createWorkPiece(Handle(AIS_Shape) model);
  void createVise();
  void cylToggle(int state);
  void enableClamping(bool enabled);
  void enableModel(bool enabled);
  void enableWorkpiece(bool enabled);
  void enableVise(bool enabled);
  void exploreModel(const TopoDS_Shape& s);
  void fixClamping();
  void fixModel();
  void fixWorkpiece();
  void fixVise();
  void loadProject(ProjectFile* pf, const TopoDS_Shape& model);
  void onTopToggle();
  void transformModel();
  void setModel(const TopoDS_Shape& shape);
  void setProject(ProjectFile* pf);
  void setupDone();
  void updateClampingPlug();
  void updateWorkPiece();
  void updateVise();

signals:
  void raiseMessage(const QString& msg);
  void modelChanged(const Bnd_Box& bb);

private:
  Ui::SetupPage*   ui;
  StringListModel* matModel;
  ViseListModel*   vises;
  };
#endif // SETUPPAGE_H
