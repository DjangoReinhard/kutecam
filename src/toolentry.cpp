/* 
 * **************************************************************************
 * 
 *  file:       toolentry.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    4.4.2022 by Django Reinhard
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
#include "toolentry.h"
#include "cuttingparameters.h"
#include <QDebug>


ToolEntry::ToolEntry(int num, const QString& name, QObject *parent)
 : QObject(parent)
 , number(num)
 , name(name)
 , cColl(0)
 , flDiam(0)
 , flLen(0)
 , cutDepth(0)
 , shankDiam(0)
 , freeLen(0)
 , cFlutes(0) {
  }


ToolEntry::ToolEntry(QObject* parent)
 : QObject(parent) {
  }


int     ToolEntry::collet() const {
  return cColl;
  }


void ToolEntry::insertCuttingParameters(int row, CuttingParameters *cp) {
  cutParams.insert(row, cp);
  }


CuttingParameters* ToolEntry::cutParameter(int row) {
  return cutParams.at(row);
  }


CuttingParameters* ToolEntry::cutParameter(const QString& material) {
  for (int i=0; i < cutParams.size(); ++i) {
      CuttingParameters* cp = cutParams.at(i);

      if (!material.compare(cp->name())) return cp;
      }
  return nullptr;
  }


QVector<CuttingParameters*>& ToolEntry::cutParameters() {
  return cutParams;
  }


const QVector<CuttingParameters*>& ToolEntry::cutParameters() const {
  return cutParams;
  }


double  ToolEntry::cuttingDepth() const {
  return cutDepth;
  }


void ToolEntry::dump() const {
  qDebug() << "\nToolEntry #" << number << "\t" << name;
  qDebug() << "\tcollet:" << cColl;
  qDebug() << "\tfl-dia:" << flDiam;
  qDebug() << "\tfl-len:" << flLen;
  qDebug() << "\tcut-Dep:" << cutDepth;
  qDebug() << "\tshank:" << shankDiam;
  qDebug() << "\tfreeL:" << freeLen;
  qDebug() << "\tnum-fl:" << cFlutes;
  for (CuttingParameters* cp : cutParams) {
      qDebug() << "\t\tmaterial:"   << cp->name();
      qDebug() << "\t\tcut-speed:"  << cp->cuttingSpeed();
      qDebug() << "\t\ttooth-feed:" << cp->toothFeed();
      qDebug() << "\t\twidth OC:" << cp->widthOfCut();
      qDebug() << "\t\tdepth OC:" << cp->depthOfCut();
      qDebug() << " ";
      }
  }


double  ToolEntry::fluteDiameter() const {
  return flDiam;
  }


double  ToolEntry::fluteLength() const {
  return flLen;
  }


double  ToolEntry::freeLength() const {
  return freeLen;
  }


bool ToolEntry::hasMaterial(const QString &material) const {
  for (auto c : cutParams) {
      if (!c->name().compare(material)) return true;
      }
  return false;
  }


double  ToolEntry::numFlutes() const {
  return cFlutes;
  }


void    ToolEntry::setCollet(int collet) {
  cColl = collet;
  }


void    ToolEntry::setFluteDiameter(double diam) {
  flDiam = diam;
  }


void    ToolEntry::setFluteLength(double len) {
  flLen = len;
  }


void    ToolEntry::setCuttingDepth(double depth) {
  cutDepth = depth;
  }


void    ToolEntry::setShankDiameter(double diam) {
  shankDiam = diam;
  }


void    ToolEntry::setFreeLength(double len) {
  freeLen = len;
  }


void    ToolEntry::setNumFlutes(double num) {
  cFlutes = num;
  }


void ToolEntry::setNumber(int number) {
  this->number = number;
  }


void ToolEntry::setToolName(const QString &name) {
  this->name = name;
  }


double  ToolEntry::shankDiameter() const {
  return shankDiam;
  }


QString ToolEntry::toolName() const {
  return name;
  }


int ToolEntry::toolNumber() const {
  return number;
  }


QString ToolEntry::toString() const {
  return QString("#%1  %2").arg(number)
                           .arg(name);
  }
