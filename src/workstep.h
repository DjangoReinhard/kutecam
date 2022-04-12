/* 
 * **************************************************************************
 * 
 *  file:       workstep.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    10.4.2022 by Django Reinhard
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
#ifndef WORKSTEP_H
#define WORKSTEP_H
#include <gp_Pnt.hxx>
#include <QString>
class QObject;
class QSettings;


enum WorkstepType
{
  WTTraverse
, WTStraightMove
, WTArc
, WTCycle
  };


class Workstep
{
public:
  explicit Workstep(WorkstepType wt, const gp_Pnt& from, const gp_Pnt& to, QObject* parent);
  explicit Workstep(WorkstepType wt, QSettings& settings, QObject* parent = nullptr);
  virtual ~Workstep() = default;

  gp_Pnt endPos() const;
  gp_Pnt startPos() const;
  WorkstepType type() const;
  virtual QString className() const;
  virtual void store(QSettings& settings);
  virtual void dump() const;

private:
  WorkstepType wsType;
  gp_Pnt       start;
  gp_Pnt       end;
  };
#endif // WORKSTEP_H
