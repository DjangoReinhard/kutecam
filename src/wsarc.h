/* 
 * **************************************************************************
 * 
 *  file:       wsarc.h
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
#ifndef WSARCCW_H
#define WSARCCW_H
#include "workstep.h"


class WSArc : public Workstep
{
public:
  explicit WSArc(const gp_Pnt& from, const gp_Pnt& to, const gp_Pnt& center, bool ccw, QObject* parent = nullptr);
  explicit WSArc(QSettings& settings, QObject* parent = nullptr);
  virtual ~WSArc() = default;

  virtual QString className() const override;
  virtual void dump() const override;
  virtual void store(QSettings& settings) override;

  bool   isCCW() const;
  gp_Pnt centerPos() const;

private:
  gp_Pnt center;
  bool   ccw;
  bool   inverted;
  };
#endif // WSARCCW_H
