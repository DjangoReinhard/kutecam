/* 
 * **************************************************************************
 * 
 *  file:       cfggcode.h
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
#ifndef CFGGCODE_H
#define CFGGCODE_H
#include <QWidget>
QT_BEGIN_NAMESPACE
namespace Ui {
class GeneralConfig;
}
QT_END_NAMESPACE
class ConfigPage;
class QSortFilterProxyModel;


class CfgGeneral : public QWidget
{
  Q_OBJECT
public:
  explicit CfgGeneral(QWidget *parent = nullptr);

public slots:
  void allInOneToggled(const QVariant& v);
  void autoRotatedChanged();
  void handleMachineType(int machineType);
  void updateMachineType();

//signals:
//  void machineTypeChanged(int machineType);

private:
  Ui::GeneralConfig*     ui;
  ConfigPage*            master;
  QSortFilterProxyModel* ppProxy;
  };
#endif // CFGGCODE_H
