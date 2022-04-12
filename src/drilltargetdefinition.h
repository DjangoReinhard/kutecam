/* 
 * **************************************************************************
 * 
 *  file:       drilltargetdefinition.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    7.4.2022 by Django Reinhard
 *  copyright:  2022 - 2022 Django Reinhard -  all rights reserved
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
#ifndef DRILLTARGETDEFINITION_H
#define DRILLTARGETDEFINITION_H
#include "targetdefinition.h"
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
class QSettings;


class DrillTargetDefinition : public TargetDefinition
{
public:
  explicit DrillTargetDefinition(const gp_Pnt& pos, const gp_Dir& dir, double radius, QObject* parent = nullptr);
  explicit DrillTargetDefinition(QSettings& settings, QObject* parent = nullptr);
  virtual ~DrillTargetDefinition() = default;

  virtual void    store(QSettings& settings) override;
  virtual QString toString() const override;

  gp_Dir dir() const { return doDir; }
  double radius() const { return doRadius; }

private:
  gp_Dir doDir;
  double doRadius;
  };
#endif // DRILLTARGETDEFINITION_H
