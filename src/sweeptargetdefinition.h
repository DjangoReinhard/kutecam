/* 
 * **************************************************************************
 * 
 *  file:       sweeptargetdefinition.h
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
#ifndef SWEEPTARGETDEFINITION_H
#define SWEEPTARGETDEFINITION_H
#include "targetdefinition.h"
#include <gp_Dir.hxx>


class SweepTargetDefinition : public TargetDefinition
{
public:
  explicit SweepTargetDefinition(const gp_Pnt& pos, const gp_Dir& dir, double radius = 0, QObject* parent = nullptr);
  explicit SweepTargetDefinition(QSettings& settings, QObject* parent = nullptr);
  virtual ~SweepTargetDefinition() = default;

  virtual void    store(QSettings& settings) override;
  virtual QString toString() const override;

  gp_Dir dir() const        { return soDir; }
  double radius() const     { return r;    }
  double zMin() const       { return zmin; }
  double zMax() const       { return zmax; }
  void   setZMin(double z)  { zmin = z; }
  void   setZMax(double z)  { zmax = z; }

private:
  gp_Dir soDir;
  double r;
  double zmin;
  double zmax;
  };
#endif // SWEEPTARGETDEFINITION_H
