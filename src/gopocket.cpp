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
#include "gocontour.h"
#include <gp_Pnt.hxx>
#include <QDebug>


//  a0: 5.38574    a1: 5.19273    from:  68.9296  /  -86.4081    to:     48.9261  /  -93.901
//  a0: 5.6052     a1: 5.33001    from:  93.2641  /  -75.1093    to:     61.3779  /  -86.4081
//  a0: 4.23205    a1: 4.03904    from: -48.9261  /  -93.901     to:    -68.9296  /  -86.4081
//  a0: 4.09477    a1: 3.81958    from: -61.3779  /  -86.4081    to:    -93.2641  /  -75.1093
//  a0: 2.50819    a1: 2.20407    from: -94.0139  /   69.0377    to:    -69.0198  /   94.0151
//  a0: 0.937526   a1: 0.6334     from:  69.0198  /   94.0151    to:     94.0139  /   69.0377
GOPocket::GOPocket(const gp_Pnt& start, const gp_Pnt& end, const gp_Pnt& center)
 : GraphicObject(GraphicType::GTPocket, start, end)
 , center(center) {
  }


GOPocket::GOPocket(const QString& s)
 : GraphicObject(GraphicType::GTPocket, s) {
  }


double GOPocket::a0() const {
  double rv = atan2(startPoint().Y() - center.Y(), startPoint().X() - center.X());

  while (rv < 0) rv += M_PI * 2;

  return rv;
  }


double GOPocket::a1() const {
  double rv = atan2(endPoint().Y() - center.Y(), endPoint().X() - center.X());

  while (rv < 0) rv += M_PI * 2;

  return rv;
  }


bool GOPocket::add(GOContour* c) {
  double a0 = c->a0();
  double a1 = c->a1();

  if (a0 < a1) c->invert();
  pool.push_back(c);

  return true;
  }


const std::vector<GOContour*>& GOPocket::contours() const {
  return pool;
  }


void GOPocket::dump() const {
  qDebug() << "==========  dump pocket ==========";
  qDebug() << "a0:" << a0() << "   a1:" << a1()
           << "   from:" << startPoint().X() << " / " << startPoint().Y()
           << "   to:   " << endPoint().X() << " / " << endPoint().Y();
  qDebug() << "contains" << pool.size() << "contours";
  for (auto& c : pool) {
      c->dump();
      }
  qDebug() << "----------  dump pocket ----------";
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


gp_Pnt GOPocket::midPoint() const {
  return center;
  }
