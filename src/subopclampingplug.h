/* 
 * **************************************************************************
 * 
 *  file:       subopclampingplug.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    28.4.2022 by Django Reinhard
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
#ifndef SUBOPCLAMPINGPLUG_H
#define SUBOPCLAMPINGPLUG_H
#include "operationsubpage.h"
class CCTargetDefinition;


class SubOPClampingPlug : public OperationSubPage
{
  Q_OBJECT
public:
  explicit SubOPClampingPlug(OperationListModel* olm, TargetDefListModel* tdModel, QWidget* parent = nullptr);
  virtual ~SubOPClampingPlug() = default;

  virtual void toolPath() override;

public slots:
  void createOP();

protected:
  int findMax(double d0, double d1, double d2, double d3);
  int findMax(int d0, int d1, int d2, int d3);
  void processSelection() override;
  void processTargets() override;
  void showToolPath() override;
  };
#endif // SUBOPCLAMPINGPLUG_H
