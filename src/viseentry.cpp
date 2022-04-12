/* 
 * **************************************************************************
 * 
 *  file:       viseentry.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    20.3.2022 by Django Reinhard
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
#include "viseentry.h"

ViseEntry::ViseEntry(QObject *parent)
 : QObject(parent) {
  }


ViseEntry::ViseEntry(const QString& name, const QString& left, const QString& right, const QString& middle, QObject* parent)
 : QObject(parent)
 , vName(name)
 , vLeft(left)
 , vMiddle(middle)
 , vRight(right) {
  }


QString ViseEntry::name() const {
  return vName;
  }


QString ViseEntry::left() const {
  return vLeft;
  }


QString ViseEntry::middle() const {
  return vMiddle;
  }


QString ViseEntry::right() const {
  return vRight;
  }


void ViseEntry::setName(const QString& name) {
  vName = name;
  }


void ViseEntry::setLeft(const QString& left) {
  vLeft = left;
  }


void ViseEntry::setMiddle(const QString& middle) {
  vMiddle = middle;
  }


void ViseEntry::setRight(const QString &right) {
  vRight = right;
  }
