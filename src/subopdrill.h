/* 
 * **************************************************************************
 * 
 *  file:       subopdrill.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    7.4.2022 by Django Reinhard
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
#ifndef SUBOPDRILL_H
#define SUBOPDRILL_H
#include "operationsubpage.h"
class PathBuilder;


class SubOPDrill : public OperationSubPage
{
  Q_OBJECT
public:
  explicit SubOPDrill(OperationListModel* olm, TargetDefListModel* tdModel, PathBuilder* pb, QWidget *parent = nullptr);
  virtual ~SubOPDrill() = default;

  virtual void genRoughingToolPath();
  virtual void genFinishingToolPath();

public slots:
  void createOP();

protected:
  void processSelection() override;
  void showToolPath(Operation* op) override;
  bool validateDrillTargets();
  };
#endif // SUBOPDRILL_H
