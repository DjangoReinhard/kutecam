/* 
 * **************************************************************************
 * 
 *  file:       subopnotch.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    14.6.2022 by Django Reinhard
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
#ifndef SUBOPNOTCH_H
#define SUBOPNOTCH_H
#include "operationsubpage.h"
#include <TopoDS_Edge.hxx>
#include <Geom_Line.hxx>
class NotchTargetDefinition;
class PathBuilder;


class SubOPNotch : public OperationSubPage
{
  Q_OBJECT
public:
  explicit SubOPNotch(OperationListModel* olm, TargetDefListModel* tdModel, PathBuilder* pb, QWidget* parent = nullptr);
  virtual ~SubOPNotch() = default;

  virtual void genRoughingToolPath();
  virtual void genFinishingToolPath();

public slots:
  void createOP();

protected:
  TopoDS_Edge determineCenterLine(const NotchTargetDefinition* ntd, Handle(Geom_Line) gl0, Handle(Geom_Line) gl1);
  void processSelection() override;
  void processTargets() override;
  };

#endif // SUBOPNOTCH_H
