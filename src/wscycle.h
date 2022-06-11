/* 
 * **************************************************************************
 * 
 *  file:       wscycle.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    10.4.2022 by Django Reinhard
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
#ifndef WSCYCLE_H
#define WSCYCLE_H
#include "workstep.h"


class WSCycle : public Workstep
{
public:
  explicit WSCycle(int cycle, const gp_Pnt& from, const gp_Pnt& to, QObject* parent = nullptr);
  explicit WSCycle(QSettings& settings, QObject* parent = nullptr);
  virtual ~WSCycle() = default;

  virtual QString className() const override;
  virtual void dump() const override;
  virtual void store(QSettings& settings) override;

  int    drillCycle() const;

private:
  int cycle;
  };
#endif // WSCYCLE_H
