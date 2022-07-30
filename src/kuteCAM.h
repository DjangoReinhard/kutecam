/* 
 * **************************************************************************
 * 
 *  file:       kuteCAM.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    11.4.2022 by Django Reinhard
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
#ifndef KUTECAM_H
#define KUTECAM_H
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <QString>
#include <cmath>


namespace kute {
extern int getDominantAxis(const gp_Dir& dir);
extern double textAsDouble(const QString& value);
extern const double  MinDelta;
extern const QString BasePath;
extern const QString CADBase;
extern const QString CADFilter;
extern const QString GCodeFilter;


inline bool isEqual(double a, double b, double minDelta = kute::MinDelta) {
  if (abs(a - b) < minDelta) return true;
  return false;
  }


inline bool isEqual(const gp_Pnt& a, const gp_Pnt& b) {
  return a.Distance(b) < kute::MinDelta;
  }


inline bool isEqual(const gp_Pnt2d& a, const gp_Pnt2d& b) {
  return a.Distance(b) < kute::MinDelta;
  }


inline bool isVertical(const gp_Dir& d) {
  if (abs(d.X()) < kute::MinDelta
   && abs(d.Y()) < kute::MinDelta
   && 1 - abs(d.Z()) < kute::MinDelta) return true;
  return false;
  }


inline bool isVertical(const gp_Vec& v) {
  if (abs(v.X()) < kute::MinDelta
   && abs(v.Y()) < kute::MinDelta
   && 1 - abs(v.Z()) < kute::MinDelta) return true;
  return false;
  }


inline double deg2rad(double deg) {
  return deg * M_PI / 180.0;
  }


inline double rad2deg(double rad) {
  return rad * 180.0 / M_PI;
  }
}
#endif // KUTECAM_H
