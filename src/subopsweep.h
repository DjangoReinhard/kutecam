/* 
 * **************************************************************************
 * 
 *  file:       subopsweep.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    18.4.2022 by Django Reinhard
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
#ifndef SUBOPSWEEP_H
#define SUBOPSWEEP_H
#include "operationsubpage.h"
#include <TopoDS_Wire.hxx>
class SweepTargetDefinition;
class OperationsListModel;
class TargetDefListModel;
class QWidget;


class SubOPSweep : public OperationSubPage
{
  Q_OBJECT
public:
  explicit SubOPSweep(OperationListModel* olm, TargetDefListModel* tdModel, QWidget* parent = nullptr);
  virtual ~SubOPSweep() = default;

  virtual void toolPath() override;

public slots:
  void createOP();

protected:
  std::vector<Handle(AIS_Shape)> createCutPlanes(Operation* op);
  void createHorizontalToolpaths(const std::vector<Handle(AIS_Shape)>& cutPlanes);
  void createRoundToolpaths(const std::vector<Handle(AIS_Shape)>& cutPlanes);
  void createVerticalToolpaths(Operation* op, Handle(AIS_Shape) cutPart);
//  Handle(AIS_Shape) genPathOffset(TopoDS_Wire wire, double off);
  void processSelection() override;
  void processTargets() override;
  void showToolPath() override;
  gp_Pnt sweepBigClockwise(const Bnd_Box& bb, const gp_Pnt& lastTO);
  gp_Pnt sweepBigCounterClockwise(const Bnd_Box& bb, const gp_Pnt& lastTO);
  };
#endif // SUBOPSWEEP_H
