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
  int                                  calcMainDir(const gp_Pnt& startPoint, const gp_Pnt& endPoint, const Bnd_Box& workBounds /* , double extend */ );
  void                                 cleanup(std::vector<Workstep*>& tp);
  std::vector<Workstep*>               genBasicPath(std::vector<std::vector<GOContour*>> clippedParts);
  std::vector<Workstep*>               genFlatPaths(Operation* op, std::vector<Handle(AIS_Shape)> cutPlanes, std::vector<std::vector<std::vector<GOContour*>>> clippedParts, double curZ, double xtend, int level = -1);
  gp_Pnt                               genInterMove(std::vector<Workstep*>& toolPath, const gp_Pnt& e, const gp_Pnt& s, const gp_Pnt& center, const Bnd_Box& workBounds, double extend);
  gp_Pnt                               genRoundInterMove(std::vector<Workstep*>& ws, const gp_Pnt& from, const gp_Pnt& to, const Bnd_Box& bb, double xtend);
  std::vector<Workstep*>               genToolPath(Operation* op, Handle(AIS_Shape) cutPart, bool wantPockets);
  std::vector<Workstep*>               genPath4Pockets(Operation* op, const Bnd_Box& bb, const gp_Dir& baseNorm, const std::vector<std::vector<GOPocket*>>& pool, double curZ, double xtend);
  std::vector<Workstep*>               genRoundToolpaths(Operation* op, const std::vector<Handle(AIS_Shape)>& cutPlanes);
  gp_Pnt                               genXTraverse(std::vector<Workstep*>& ws, int dir, const gp_Pnt& startPos, const gp_Pnt& endPos, const Bnd_Box& bb /*, double xtend */);
  gp_Pnt                               genYTraverse(std::vector<Workstep*>& ws, int dir, const gp_Pnt& startPos, const gp_Pnt& endPos, const Bnd_Box& bb /*, double xtend */);
  gp_Pnt                               processContour(std::vector<Workstep*>& toolPath, GOContour* c);
  std::vector<std::vector<GOContour*>> processCurve(Operation* op, GOContour* curve, bool curveIsBorder, const gp_Pnt& center, /* double extend, */ double firstOffset, double curZ);
  int                                  quadrant(const gp_Pnt& p, const gp_Pnt& center = {0,0,0}) const;
  int                                  region(const gp_Pnt& p, const Bnd_Box& bb) const;
  void                                 simplify(std::vector<GOContour*>& pool);
  std::vector<std::vector<GOPocket*>>  splitCurves(const Operation* op, const std::vector<std::vector<std::vector<GOContour*>>>& pool);
  void                                 stripPath(GOContour* firstContour, GOContour* masterContour);

protected:
  void drawDebugContour(Operation* op, GOContour* c, double z);

  static const int Inside;
  static const int Left;
  static const int Right;
  static const int Bottom;
  static const int Top;
  };
#endif // PATHBUILDER_H
