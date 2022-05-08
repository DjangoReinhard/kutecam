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
