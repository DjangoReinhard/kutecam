/* 
 * **************************************************************************
 * 
 *  file:       pocketpathbuilder.cpp
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
#include "pocketpathbuilder.h"
#include "core.h"
#include "gocontour.h"
#include "gopocket.h"
#include "operation.h"
#include "pathbuilderutil.h"
#include "work.h"
#include "wstraverse.h"
#include <Bnd_Box.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>


PocketPathBuilder::PocketPathBuilder(PathBuilderUtil* pbu)
 : pbu(pbu) {
  assert(pbu);
  }


std::vector<Workstep*> PocketPathBuilder::genPath(Operation* op, const Bnd_Box& bb, const gp_Dir& baseNorm, const std::vector<std::vector<GOPocket*>>& pool, double curZ, double xtend) {
  double radius = 0;
  gp_Pnt center(bb.CornerMin().X() + (bb.CornerMax().X() - bb.CornerMin().X()) / 2
              , bb.CornerMin().Y() + (bb.CornerMax().Y() - bb.CornerMin().Y()) / 2
              , bb.CornerMin().Z() + (bb.CornerMax().Z() - bb.CornerMin().Z()) / 2);
  bool roundWorkPiece = Core().workData()->roundWorkPiece;
  std::vector<Workstep*> toolPath;
  int iMin = 0, iMax = 99;
  gp_Pnt  s(0, 0, 300);
  gp_Pnt  e = s, tmp;

  for (auto levelParts : pool) {
      int mxI = fmin(iMax, levelParts.size());

      for (int i=iMin; i < mxI; ++i) {
          GOPocket* p = levelParts.at(i);
          int mx = p->contours().size();

          p->dump();
          for (int j=0; j < mx; ++j) {
              GOContour* c   = p->contours().at(j);

              if (c->isClosed()) c->changeStart2Close(e);
              c->simplify(curZ);
              s = c->startPoint();
              if (roundWorkPiece) pbu->genRoundInterMove(toolPath, e, s, bb, xtend + i + j);
              else                pbu->genInterMove(toolPath, e, s, center, bb, xtend + i + j);
//              drawDebugContour(op, c, curZ);
              pbu->processContour(toolPath, c);
              e = c->endPoint();
              }
          s = e;
          e.SetZ(bb.CornerMax().Z() + op->safeZ1());
          if (!kute::isEqual(s, e)) toolPath.push_back(new WSTraverse(s, e));
          }
      curZ -= op->cutDepth();
      }
  pbu->cleanup(toolPath);

  return toolPath;
  }
