/* 
 * **************************************************************************
 * 
 *  file:       ppSinumeric840D.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    17.5.2022 by Django Reinhard
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
#ifndef PPSINUMERIC840D_H
#define PPSINUMERIC840D_H
#include <dinpostprocessor.h>


class PPSinumeric840D : public DINPostProcessor
{
  Q_OBJECT
  Q_INTERFACES(PostProcessor)
#ifdef USE_PLUGINS
  Q_PLUGIN_METADATA(IID "PostProcessorPlugin_iid" FILE "ppSinumeric840D.json")
#endif
public:
  explicit PPSinumeric840D(QObject* parent = nullptr);
  virtual ~PPSinumeric840D() = default;

  virtual QString fixtureID(int f) override;
  virtual QString genDefineCycle(int c, double topZ, double r0, double r1, double depth, double qMin, double qMax, double retract, double dwell, int feed) override;
  virtual QString genEndCycle() override;
  virtual QString getFileExtension() const override;
  virtual QString genLengthCorrEnd() override;
  virtual QString genLengthCorrStart(int toolNum) override;
  virtual QString genPrepareTool(int toolNum) override;
  virtual QString genToolChange() override;
  };
#endif
