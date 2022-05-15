/* 
 * **************************************************************************
 * 
 *  file:       cfgvise.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    15.5.2022 by Django Reinhard
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
#ifndef CFGVISE_H
#define CFGVISE_H
#include <QWidget>
#include <AIS_Shape.hxx>
QT_BEGIN_NAMESPACE
namespace Ui {
class ViseConfig;
}
QT_END_NAMESPACE
class ViseListModel;
class QItemSelection;

class CfgVise : public QWidget
{
  Q_OBJECT
public:
  explicit CfgVise(ViseListModel* vModel, QWidget *parent = nullptr);

public slots:
  void addVise();
  void delVise();
  void setLeftVise();
  void setMidVise();
  void setRightVise();
  void viseSelected(const QItemSelection &selected, const QItemSelection &deselected);

private:
  Ui::ViseConfig*   ui;
  ViseListModel*    vModel;
  Handle(AIS_Shape) leftVise;
  Handle(AIS_Shape) midVise;
  Handle(AIS_Shape) rightVise;
  };
#endif // CFGVISE_H
