/* 
 * **************************************************************************
 * 
 *  file:       notchtargetdefinition.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    21.6.2022 by Django Reinhard
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
#ifndef NOTCHTARGETDEFINITION_H
#define NOTCHTARGETDEFINITION_H
#include "targetdefinition.h"
#include <gp_Pln.hxx>
#include <Geom_Curve.hxx>


class NotchTargetDefinition : public TargetDefinition
{
  Q_OBJECT
public:
  explicit NotchTargetDefinition(gp_Pln face, const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, QObject *parent = nullptr);
  explicit NotchTargetDefinition(QSettings& settings, QObject* parent = nullptr);
  virtual ~NotchTargetDefinition() = default;

  gp_Pln             bottom() const;
  gp_Pnt             borderPoint(int index) const;
  void               store(QSettings& s) override;
  QString            toString() const override;

private:
  gp_Pln             btmFace;
  gp_Pnt             bp[4];
  };
#endif // NOTCHTARGETDEFINITION_H
