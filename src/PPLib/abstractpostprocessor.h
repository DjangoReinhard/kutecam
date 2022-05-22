/* 
 * **************************************************************************
 * 
 *  file:       abstractpostprocessor.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    19.5.2022 by Django Reinhard
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
#ifndef ABSTRACTPOSTPROCESSOR_H
#define ABSTRACTPOSTPROCESSOR_H
#include "postprocessor.h"
#include "toolentry.h"
#include <gp_Pnt.hxx>


class AbstractPostProcessor : public QObject
{
  Q_OBJECT
public:
  explicit AbstractPostProcessor(QObject* parent = nullptr);
  virtual ~AbstractPostProcessor() = default;

protected:
  gp_Pnt    lPos;
  gp_Pnt    rot;
  ToolEntry curTool;
  int       radiusCorr;
  int       feed;

  const double MinDelta = 1e-5;
  const int    Decimals = 3;
  };
#endif // ABSTRACTPOSTPROCESSOR_H
