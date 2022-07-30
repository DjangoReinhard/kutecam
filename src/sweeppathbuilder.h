/* 
 * **************************************************************************
 * 
 *  file:       sweeppathbuilder.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    15.7.2022 by Django Reinhard
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
#ifndef SWEEPPATHBUILDER_H
#define SWEEPPATHBUILDER_H
#include <AIS_Shape.hxx>
#include <vector>
class Operation;
class PathBuilderUtil;
class ToolEntry;


class SweepPathBuilder
{
public:
  SweepPathBuilder(PathBuilderUtil* pbu);

  void createHorizontalToolpaths(Operation* op, const std::vector<Handle(AIS_Shape)>& cutPlanes);

protected:
  gp_Pnt sweepBigClockwise(Operation* op, ToolEntry* activeTool, const Bnd_Box& bb, const gp_Pnt& lastTO);
  gp_Pnt sweepBigCounterClockwise(Operation* op, ToolEntry* activeTool, const Bnd_Box& bb, const gp_Pnt& lastTO);

private:
  PathBuilderUtil* pbu;
  };
#endif // SWEEPPATHBUILDER_H
