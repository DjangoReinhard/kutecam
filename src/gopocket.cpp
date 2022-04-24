/* 
 * **************************************************************************
 * 
 *  file:       gopocket.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    21.4.2022 by Django Reinhard
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
#include "gopocket.h"
#include <gp_Pnt.hxx>


GOPocket::GOPocket(double from, double to)
 : GraphicObject(GraphicType::GTPocket, gp_Pnt(from, 0, 0), gp_Pnt(to, 0, 0)) {
  }


GOPocket::GOPocket(const QString& s)
 : GraphicObject(GraphicType::GTPocket, s) {
  }


double GOPocket::a0() const {
  return startPoint().X();
  }


double GOPocket::a1() const {
  return endPoint().X();
  }


std::vector<GOContour*>& GOPocket::contours() {
  return pool;
  }


GraphicObject* GOPocket::extendBy(double length) {
  return this;
  }


GraphicObject* GOPocket::extendStart(double length) {
  return this;
  }


GraphicObject* GOPocket::extendEnd(double length) {
  return this;
  }


GraphicObject* GOPocket::invert() {

  return this;
  }


bool compPocket(GOPocket* left, GOPocket* right) {
  if (!left || !right) return false;
  return left->contours().size() < right->contours().size();
  }
