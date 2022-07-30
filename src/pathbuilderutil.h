/* 
 * **************************************************************************
 * 
 *  file:       pathbuilderutil.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    26.7.2022 by Django Reinhard
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
#ifndef PATHBUILDERUTIL_H
#define PATHBUILDERUTIL_H
#include <vector>
#include <gp_Pnt.hxx>
class Bnd_Box;
class Workstep;
class GOContour;
class PocketPathBuilder;
class ProfitMillingBuilder;
class SweepPathBuilder;


class PathBuilderUtil
{
public:
  PathBuilderUtil();

  void   cleanup(std::vector<Workstep*>& toolPath);
  gp_Pnt genRoundInterMove(std::vector<Workstep*>& ws, const gp_Pnt& from, const gp_Pnt& to, const Bnd_Box& bb, double xtend);
  gp_Pnt genInterMove(std::vector<Workstep*>& toolPath, const gp_Pnt& e, const gp_Pnt& s, const gp_Pnt& center, const Bnd_Box& workBounds, double extend);
  gp_Pnt processContour(std::vector<Workstep*>& toolPath, GOContour* c);
  PocketPathBuilder* pocketPathBuilder();
  ProfitMillingBuilder* profitMillingBuilder();
  SweepPathBuilder*  sweepPathBuilder();

protected:
  int quadrant(const gp_Pnt& p, const gp_Pnt& center = {0,0,0}) const;
  int region(const gp_Pnt& p, const Bnd_Box& bb) const;

  static const int Inside;
  static const int Left;
  static const int Right;
  static const int Bottom;
  static const int Top;

private:
  PocketPathBuilder*    ppb;
  ProfitMillingBuilder* pmb;
  SweepPathBuilder*     spb;
  };

#endif // PATHBUILDERUTIL_H
