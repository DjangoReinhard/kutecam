/* 
 * **************************************************************************
 * 
 *  file:       cctargetdefinition.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    28.4.2022 by Django Reinhard
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
#ifndef CCTARGETDEFINITION_H
#define CCTARGETDEFINITION_H
#include "targetdefinition.h"
class GOContour;


class CCTargetDefinition : public TargetDefinition
{
public:
  explicit CCTargetDefinition(const gp_Pnt& pMin, const gp_Pnt& pMax, QObject* parent = nullptr);
  explicit CCTargetDefinition(QSettings& settings, QObject* parent = nullptr);
  virtual ~CCTargetDefinition() = default;

  gp_Pnt cornerMin() const { return pos(); };
  gp_Pnt cornerMax() const { return pMax;  };

  virtual void    store(QSettings& settings) override;
  virtual QString toString() const override;

private:
  gp_Pnt pMax;
  };
#endif // CCTARGETDEFINITION_H
