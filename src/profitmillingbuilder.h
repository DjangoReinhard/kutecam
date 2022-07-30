/* 
 * **************************************************************************
 * 
 *  file:       profitmillingbuilder.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    30.7.2022 by Django Reinhard
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
#ifndef PROFITMILLINGBUILDER_H
#define PROFITMILLINGBUILDER_H
#include <AIS_Shape.hxx>
#include <Geom_Line.hxx>
#include <TopoDS_Edge.hxx>
class NotchTargetDefinition;
class Operation;
class PathBuilderUtil;
class Workstep;


class ProfitMillingBuilder
{
public:
  ProfitMillingBuilder(PathBuilderUtil* pbu);

  std::vector<Workstep*> genToolPath(Operation* op, Handle(AIS_Shape) cutPart, std::vector<Handle(AIS_Shape)> cutPlanes);
  TopoDS_Edge determineCenterLine(const NotchTargetDefinition* ntd, Handle(Geom_Line) gl0, Handle(Geom_Line) gl1, TopoDS_Shape cutPart);

private:
  PathBuilderUtil* pbu;
  };
#endif // PROFITMILLINGBUILDER_H
