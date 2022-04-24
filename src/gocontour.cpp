/* 
 * **************************************************************************
 * 
 *  file:       gocontour.cpp
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
#include "gocontour.h"
#include "gocircle.h"
#include "goline.h"
#include "core.h"
#include "util3d.h"
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRep_Tool.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <TopoDS.hxx>
#include <QDebug>


GOContour::GOContour(const gp_Pnt& center)
 : GraphicObject(GraphicType::GTContour, gp_Pnt(), gp_Pnt())
 , center(center) {
  }


GOContour::GOContour(const QString& s)
  : GraphicObject(GraphicType::GTContour, s) {
  QStringList    sl = s.split("|");
  GraphicObject* go;
  bool           ok;

  for (int i=1; i < sl.size(); ++i) {
      const QString& src = sl.at(i);

      go = Core().helper3D()->parseGraphicObject(src);
      if (go) add(go);
      }
  }


bool GOContour::add(GraphicObject* o) {
  if (!o) return false;
  if (!segs.size()) {
     segs.push_back(o);
     setStartPoint(o->startPoint());
     setEndPoint(o->endPoint());

     return true;
     }
  if (Core().helper3D()->isEqual(o->endPoint(), endPoint())
   || Core().helper3D()->isEqual(o->startPoint(), startPoint()))
     o->invert();

  if (Core().helper3D()->isEqual(o->startPoint(), endPoint())) {
     segs.push_back(o);
     setEndPoint(o->endPoint());

     return true;
     }
  if (Core().helper3D()->isEqual(o->endPoint(), startPoint())) {
     segs.insert(segs.begin(), o);
     setStartPoint(o->startPoint());

     return true;
     }
  return false;
  }


bool GOContour::add(GOContour *other) {
  if (!other) return false;
  if (Core().helper3D()->isEqual(other->startPoint(), startPoint())) {
     auto& v = other->segments();

     for (auto i = v.rbegin(); i != v.rend(); ++i)
         add(*i);
     delete other;

     return true;
     }
  else if (Core().helper3D()->isEqual(other->startPoint(), endPoint())) {
     auto& v = other->segments();

     for (auto i = v.begin(); i != v.end(); ++i)
         add(*i);
     delete other;

     return true;
     }
  else if (Core().helper3D()->isEqual(other->endPoint(), startPoint())) {
     const auto& v = other->segments();

     for (auto i = v.rbegin(); i != v.rend(); ++i)
         add(*i);
     delete other;

     return true;
     }
  else if (Core().helper3D()->isEqual(other->endPoint(), endPoint())) {
     auto& v = other->segments();

     for (auto i = v.begin(); i != v.end(); ++i)
         add(*i);
     delete other;

     return true;
     }
  return false;
  }


GraphicObject* GOContour::add(TopoDS_Shape s) {
  const TopoDS_Edge e  = TopoDS::Edge(s);
  GraphicObject*    rv = nullptr;

  if (!e.IsNull()) {
     double             param0, param1;
     Handle(Geom_Curve) c = BRep_Tool::Curve(e, param0, param1);
     gp_Pnt             p0, p1;

     p0 = c->Value(param0);
     p1 = c->Value(param1);

     if (c->DynamicType() == STANDARD_TYPE(Geom_Line)) {
        qDebug() << "segment is LINE from" << p0.X() << " / " << p0.Y() << " / " << p0.Z()
                 << " to "                 << p1.X() << " / " << p1.Y() << " / " << p1.Z();
        rv = new GOLine(p0, p1);
        }
     else if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
        gp_Pnt pM = c->Value((param0 + param1) / 2);

        qDebug() << "segment is CIRCLE from" << p0.X() << " / " << p0.Y() << " / " << p0.Z()
                 << " to "                   << p1.X() << " / " << p1.Y() << " / " << p1.Z()
                 << " with center"           << pM.X() << " / " << pM.Y() << " / " << pM.Z();
        rv = new GOCircle(p0, p1, pM);
        }
     else {
        throw std::domain_error("unsupported Geom-Type!");
        }
     if (rv) {
        if (!segs.size()) {
           segs.push_back(rv);
           setStartPoint(rv->startPoint());
           setEndPoint(rv->endPoint());

           return nullptr;
           }
        else {
           if (Core().helper3D()->isEqual(endPoint(), rv->startPoint())) {
              segs.push_back(rv);
              setEndPoint(rv->endPoint());

              return nullptr;
              }
           else if (Core().helper3D()->isEqual(endPoint(), rv->endPoint())) {
              rv->invert();
              segs.push_back(rv);
              setEndPoint(rv->endPoint());

              return nullptr;
              }
           else {
              if (Core().helper3D()->isEqual(startPoint(), rv->startPoint())) {
                 rv->invert();
                 segs.insert(segs.begin(), rv);
                 setStartPoint(rv->startPoint());

                 return nullptr;
                 }
              else if (Core().helper3D()->isEqual(startPoint(), rv->endPoint())) {
                 segs.insert(segs.begin(), rv);
                 setStartPoint(rv->startPoint());

                 return nullptr;
                 }
              else {
                 qDebug() << "given segment does not fit on either side of contour. append() rejected!";
                 }
              }
           }
        }
     }
  return rv;
  }


double GOContour::angEnd() const {
  return atan2(endPoint().Y() - center.Y(), endPoint().X() - center.X());
  }


double GOContour::angStart() const {
  return atan2(startPoint().Y() - center.Y(), startPoint().X() - center.X());
  }


GraphicObject& GOContour::extendBy(double length) {
  extendStart(length);
  extendEnd(length);

  return *this;
  }


GraphicObject* GOContour::extendEnd(double length) {
  GraphicObject* go = segs.at(segs.size() - 1);

  if (go->type() == GTCircle) {
     GOCircle*  gc = static_cast<GOCircle*>(go);
     double fp, lp;
     Handle(Geom_Curve) c = gc->endTangent(length, &fp, &lp);
     gp_Pnt sp = c->Value(fp);
     gp_Pnt ep = c->Value(lp);

     segs.push_back(new GOLine(sp, ep));
     setEndPoint(ep);
     }
  else {
     go->extendEnd(length);
     setEndPoint(go->endPoint());
     }
  return this;
  }


GraphicObject* GOContour::extendStart(double length) {
  GraphicObject* go = segs.at(0);

  if (go->type() == GTCircle) {
     GOCircle*  gc = static_cast<GOCircle*>(go);
     double fp, lp;
     Handle(Geom_Curve) c = gc->startTangent(length, &fp, &lp);
     gp_Pnt sp = c->Value(fp);
     gp_Pnt ep = c->Value(lp);

     segs.insert(segs.begin(), new GOLine(sp, ep));
     setStartPoint(sp);
     }
  else if (go->type() == GTLine) {
     go->extendStart(length);

     setStartPoint(go->startPoint());
     }
  return this;
  }


GraphicObject* GOContour::invert() {
  auto first = segs.begin();
  auto last  = segs.end();
  int  mx    = segs.size();

  while ((first != last) && (first != --last)) {
        std::iter_swap(first, last);
        (*first)->invert();
        (*last)->invert();
        ++first;
        }
  if (mx % 2) {      // center element is not handled by above invert loop
     segs.at(mx / 2)->invert();
     }
  setStartPoint(segs.at(0)->startPoint());
  setEndPoint(segs.at(mx - 1)->endPoint());

  return this;
  }


std::vector<GraphicObject*>& GOContour::segments() {
  return segs;
  }


void GOContour::setContour(TopoDS_Shape contour) {
  std::vector<TopoDS_Edge> segments = Core().helper3D()->allEdgesWithin(contour);

  segs.clear();
  for (auto s : segments)
      add(s);
  }


std::vector<GraphicObject*>& GOContour::simplify(double z) {
  int mx = segs.size();

  if (segs.size() > 2) {
     for (int i=0; i < mx; ++i) {
         GraphicObject* o0 = segs.at(i);
         GraphicObject* o1 = segs.at((i+1) < mx ? (i+1) : 0);

         if (o0->type() == GTLine && o1->type() == GTLine) {
            GOLine* l0 = static_cast<GOLine*>(o0);
            GOLine* l1 = static_cast<GOLine*>(o1);

            l0->setZ(z);
            l1->setZ(z);
            double  p2 = l0->p1 + (l1->p1 - l1->p0);

            if (Core().helper3D()->isEqual(l0->curve->Value(p2), l1->endPoint())) {
               l0->extendEnd(l1->p1 - l1->p0);
               if ((i+1) < mx) {
                  segs.erase(segs.begin() + i + 1);
                  }
               else {
                  setEndPoint(o0->endPoint());
                  segs.erase(segs.begin());
                  setStartPoint(segs.at(0)->startPoint());
                  }
               mx = segs.size();
               }
            }
         else o0->setZ(z);
         }
     setStartPoint(segs.at(0)->startPoint());
     setEndPoint(segs.at(segs.size() - 1)->endPoint());
     }
  else if (segs.size() == 1) {
     // need at least 2 segments to form a wire for offset paths
     GraphicObject* go = segs.at(0);

     go->setZ(z);
     setStartPoint(go->startPoint());
     setEndPoint(go->endPoint());
     add(segs.at(0)->split());
     }
  return segs;
  }


int GOContour::size() const {
  return segs.size();
  }


TopoDS_Shape GOContour::toWire(double z) {
  BRepBuilderAPI_MakeWire wireBuilder;

  for (int n=0; n < segs.size(); ++n) {
      auto go = segs.at(n);
      TopoDS_Shape s = go->toShape(z)->Shape();

      Core().shapeFix().SetTolerance(s, Core().MinDelta);
      wireBuilder.Add(TopoDS::Edge(s));
      if (wireBuilder.Error()) break;
      }
  return wireBuilder.Wire();
  }


Handle(AIS_Shape) GOContour::toShape(double z) {
  return new AIS_Shape(toWire(z));
  }


QString GOContour::toString() const {
  QString rv = GraphicObject::toString();

  for (auto s : segs) {
      rv += "|";
      rv += s->toString();
      }
  return rv;
  }


bool compContour(GOContour* left, GOContour* right) {
  if (!left || !right) return false;
  return left->angStart() < right->angStart();
  }
