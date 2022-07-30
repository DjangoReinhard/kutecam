/* 
 * **************************************************************************
 * 
 *  file:       subop3dface.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    16.6.2022 by Django Reinhard
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
#ifndef SUBOP3DFACE_H
#define SUBOP3DFACE_H
#include "operationsubpage.h"
class PathBuilder;


class SubOP3DFace : public OperationSubPage
{
  Q_OBJECT
public:
  explicit SubOP3DFace(OperationListModel* olm, TargetDefListModel* tdModel, PathBuilder* pb, QWidget* parent = nullptr);
  virtual ~SubOP3DFace() = default;

  virtual void genRoughingToolPath();
  virtual void genFinishingToolPath();

public slots:
  void createOP();

protected:
  void processSelection() override;
  void processTargets() override;
  };
#endif // SUBOP3DFACE_H
