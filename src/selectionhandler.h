/* 
 * **************************************************************************
 * 
 *  file:       selectionhandler.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    23.4.2022 by Django Reinhard
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
#ifndef SELECTIONHANDLER_H
#define SELECTIONHANDLER_H
#include <TopoDS_Wire.hxx>
#include <AIS_Shape.hxx>
class GOContour;
class Operation;
class SweepTargetDefinition;


class SelectionHandler
{
public:
  SelectionHandler();

  TopoDS_Shape      createBaseContour(const gp_Pnt& pos, const gp_Dir& dir, Operation* op);
  GOContour*        createContourFromSelection(Operation* op, Bnd_Box* pBB = nullptr);
  Handle(AIS_Shape) createCutPart(TopoDS_Shape cf, Operation* op, bool wantFirst = true);
  Handle(AIS_Shape) createCutPart(Operation* op);
  Handle(AIS_Shape) createCutPart(Operation* op, SweepTargetDefinition* std);
  };
#endif // SELECTIONHANDLER_H
