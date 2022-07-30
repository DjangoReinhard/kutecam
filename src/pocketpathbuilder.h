/* 
 * **************************************************************************
 * 
 *  file:       pocketpathbuilder.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    11.7.2022 by Django Reinhard
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
#ifndef POCKETPATHBUILDER_H
#define POCKETPATHBUILDER_H
#include <vector>
class Bnd_Box;
class gp_Dir;
class GOPocket;
class Workstep;
class Operation;
class PathBuilderUtil;


class PocketPathBuilder
{
public:
  PocketPathBuilder(PathBuilderUtil* pbu);

  std::vector<Workstep*> genPath(Operation* op, const Bnd_Box& bb, const gp_Dir& baseNorm, const std::vector<std::vector<GOPocket*>>& pool, double curZ, double xtend);

private:
  PathBuilderUtil* pbu;
  };
#endif // POCKETPATHBUILDER_H
