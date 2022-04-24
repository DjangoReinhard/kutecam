/* 
 * **************************************************************************
 * 
 *  file:       sweeptargetdefinition.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    24.4.2022 by Django Reinhard
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
#ifndef SWEEPTARGETDEFINITION_H
#define SWEEPTARGETDEFINITION_H
#include "targetdefinition.h"
#include <Bnd_Box.hxx>
#include <gp_Dir.hxx>
class GOContour;


class SweepTargetDefinition : public TargetDefinition
{
public:
  explicit SweepTargetDefinition(const gp_Pnt& pos, const gp_Dir& dir, double radius = 0, QObject* parent = nullptr);
  explicit SweepTargetDefinition(GOContour* c, QObject* parent = nullptr);
  explicit SweepTargetDefinition(QSettings& settings, QObject* parent = nullptr);
  virtual ~SweepTargetDefinition() = default;

  virtual void    store(QSettings& settings) override;
  virtual QString toString() const override;

  Bnd_Box    baseDelim() const               { return bbBase; }
  gp_Dir     baseDir() const                 { return dirBase; }
  GOContour* contour() const                 { return cc; }
  gp_Dir     dir() const                     { return soDir; }
  bool       isBaseBorder() const            { return baseIsBorder; }
  double     radius() const                  { return r;    }
  void       setBaseDelim(const Bnd_Box& bb) { bbBase = bb; }
  void       setContour(GOContour* c)        { cc = c; }
  void       setBaseDir(const gp_Dir& nd)    { dirBase = nd; }
  void       setBaseIsBorder(bool bib)       { baseIsBorder = bib; }
  void       setZMin(double z)               { zmin = z; }
  void       setZMax(double z)               { zmax = z; }
  double     zMin() const                    { return zmin; }
  double     zMax() const                    { return zmax; }

private:
  gp_Dir     soDir;
  gp_Dir     dirBase;
  Bnd_Box    bbBase;
  bool       baseIsBorder;
  GOContour* cc;
  double     r;
  double     zmin;
  double     zmax;
  };
#endif // SWEEPTARGETDEFINITION_H
