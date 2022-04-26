/* 
 * **************************************************************************
 * 
 *  file:       targetdefinition.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    23.4.2022 by Django Reinhard
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
#ifndef TARGETDEFINITION_H
#define TARGETDEFINITION_H
#include <QObject>
#include <gp_Pnt.hxx>
class QSettings;
class GOContour;


class TargetDefinition : public QObject
{
  Q_OBJECT
public:
  explicit TargetDefinition(const gp_Pnt& pos, double radius, QObject* parent = nullptr);
  explicit TargetDefinition(QSettings& settings, QObject* parent = nullptr);
  virtual ~TargetDefinition() = default;

  virtual void    store(QSettings& settings);
  virtual QString toString() const = 0;

  GOContour* contour() const { return cc; }
  gp_Pnt pos() const  { return tdPos; }
  double radius() const { return r; }
  double zMin() const { return zmin; }
  double zMax() const { return zmax; }
  void   setContour(GOContour* c) { cc = c; }
  void   setRadius(double radius) { r = radius; }
  void   setZMin(double z) { zmin = z; }
  void   setZMax(double z) { zmax = z; }

  static bool     compareASC(TargetDefinition* left, TargetDefinition* right);
  static bool     compareDESC(TargetDefinition* left, TargetDefinition* right);

private:
  gp_Pnt     tdPos;
  double     zmin;
  double     zmax;
  double     r;
  GOContour* cc;
  };
#endif // TARGETDEFINITION_H
