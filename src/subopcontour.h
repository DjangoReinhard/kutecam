/* 
 * **************************************************************************
 * 
 *  file:       subopcontour.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    16.4.2022 by Django Reinhard
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
#ifndef SUBOPCONTOUR_H
#define SUBOPCONTOUR_H
#include "operationsubpage.h"
class PathBuilder;


class SubOPContour : public OperationSubPage
{
  Q_OBJECT
public:
  explicit SubOPContour(OperationListModel* olm, TargetDefListModel* tdModel, PathBuilder* pb, QWidget *parent = nullptr);
  virtual ~SubOPContour() = default;

  virtual void genRoughingToolPath();
  virtual void genFinishingToolPath();
  virtual void processTargets() override;

public slots:
  void createOP();
  void updateCut(double d);

protected:
  void processSelection();
  };
#endif // SUBOPCONTOUR_H
