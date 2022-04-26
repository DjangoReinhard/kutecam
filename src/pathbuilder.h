/* 
 * **************************************************************************
 * 
 *  file:       pathbuilder.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    24.4.2022 by Django Reinhard
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
#ifndef PATHBUILDER_H
#define PATHBUILDER_H
#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <vector>
class GOContour;
class GOPocket;
class Operation;
class SweepTargetDefinition;
class Workstep;


class PathBuilder
{
public:
  PathBuilder();

  double                               calcAdditionalOffset(SweepTargetDefinition* std, GOContour* c);
  int                                  calcSafeOffsets(double& safeX, double& safeY, const gp_Dir& baseNorm, Operation* op, double xtend);
  std::vector<Workstep*>               genToolPath(Operation* op);
  std::vector<Workstep*>               genToolPath(Operation* op, Handle(AIS_Shape) cutPart);
  std::vector<Workstep*>               genPath4Pockets(Operation* op, const Bnd_Box& bb, const gp_Dir& baseNorm, const std::vector<std::vector<GOPocket*>>& pool, double xtend);
  std::vector<Workstep*>               genRoundToolpaths(Operation* op, const std::vector<Handle(AIS_Shape)>& cutPlanes);
  std::vector<std::vector<GOContour*>> processCurve(Operation* op, GOContour* curve, bool curveIsBorder, const gp_Pnt& center, double extend, double firstOffset, double curZ);
  std::vector<std::vector<GOPocket*>>  splitCurves(const std::vector<std::vector<std::vector<GOContour*>>>& pool);
  void                                 stripPath(GOContour* firstContour, GOContour* masterContour);
  };
#endif // PATHBUILDER_H
