/* 
 * **************************************************************************
 * 
 *  file:       cuttingparameters.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    23.3.2022 by Django Reinhard
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
#include "cuttingparameters.h"


CuttingParameters::CuttingParameters(const QString& name, QObject *parent)
 : QObject(parent)
 , material(name)
 , cutSpeed(0)
 , tFeed(0)
 , wOC(0)
 , dOC(0) {
  }


CuttingParameters::CuttingParameters(QObject* parent)
 : QObject(parent) {
  }


double  CuttingParameters::cuttingSpeed() const {
  return cutSpeed;
  }


double  CuttingParameters::toothFeed() const {
  return tFeed;
  }


double  CuttingParameters::widthOfCut() const {
  return wOC;
  }


double  CuttingParameters::depthOfCut() const {
  return dOC;
  }


QString CuttingParameters::name() const {
  return material;
  }


void    CuttingParameters::setCuttingSpeed(double speed) {
  cutSpeed = speed;
  }


void    CuttingParameters::setName(const QString &name) {
  material = name;
  }


void    CuttingParameters::setToothFeed(double feed) {
  tFeed = feed;
  }


void    CuttingParameters::setWidthOfCut(double width) {
  wOC = width;
  }


void    CuttingParameters::setDepthOfCut(double depth) {
  dOC = depth;
  }
