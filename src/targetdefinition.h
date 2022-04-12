/* 
 * **************************************************************************
 * 
 *  file:       targetdefinition.h
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
#ifndef TARGETDEFINITION_H
#define TARGETDEFINITION_H
#include <QObject>
#include <gp_Pnt.hxx>
class QSettings;


class TargetDefinition : public QObject
{
  Q_OBJECT
public:
  explicit TargetDefinition(const gp_Pnt& pos, QObject* parent = nullptr);
  explicit TargetDefinition(QSettings& settings, QObject* parent = nullptr);
  virtual ~TargetDefinition() = default;

  virtual void    store(QSettings& settings);
  virtual QString toString() const = 0;

  gp_Pnt pos()    const { return tdPos; }

protected:
  gp_Pnt tdPos;
  };
#endif // TARGETDEFINITION_H
