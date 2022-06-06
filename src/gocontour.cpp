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
#include "kuteCAM.h"
#include "util3d.h"
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRep_Tool.hxx>
#include <ElCLib.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <TopoDS.hxx>
#include <QDebug>
#include <cmath>


GOContour::GOContour(const gp_Pnt& center, int order)
 : GraphicObject(GraphicType::GTContour, gp_Pnt(), gp_Pnt())
 , center(center)
 , level(order) {
  }


GOContour::GOContour(const QString& s)
  : GraphicObject(GraphicType::GTContour, s)
  , level(0) {
  QStringList    sls   = s.split("|");
  QStringList    sl    = sls.at(0).split(";");
  QStringList    subSL = sl.at(3).split("/");
  bool           ok    = false;
  double         x, y, z;
  GraphicObject* go;

  x = subSL.at(0).toDouble(&ok);
  y = subSL.at(1).toDouble(&ok);
  z = subSL.at(2).toDouble(&ok);  
  center = gp_Pnt(x, y, z);

  if (sl.size() > 4) level = sl.at(4).toInt();
  for (int i=1; i < sls.size(); ++i) {
      const QString& src = sls.at(i);

      go = Core().helper3D()->parseGraphicObject(src);
      if (go) add(go);
      }
  }


double GOContour::a0() const {
  double rv = atan2(startPoint().Y() - center.Y(), startPoint().X() - center.X());

  while (rv < 0) rv += M_PI * 2;

  return rv;
  }


double GOContour::a1() const {
  double rv = atan2(endPoint().Y() - center.Y(), endPoint().X() - center.X());

  while (rv < 0) rv += M_PI * 2;

  return rv;
  }


bool GOContour::add(GraphicObject* o) {
  if (!o) return false;
  if (!segs.size()) {
     segs.push_back(o);
     setStartPoint(o->startPoint());
     setEndPoint(o->endPoint());

     return true;
     }
  GraphicObject* first = segs.at(0);
  GraphicObject* last  = segs.at(segs.size() - 1);

  if (kute::isEqual(o->endPoint(), last->endPoint())
   || kute::isEqual(o->startPoint(), first->startPoint()))
     o->invert();

  if (kute::isEqual(o->startPoint(), last->endPoint())) {
     segs.push_back(o);
     setEndPoint(o->endPoint());

     return true;
     }
  if (kute::isEqual(o->endPoint(), first->startPoint())) {
     segs.insert(segs.begin(), o);
     setStartPoint(o->startPoint());

     return true;
     }
  return false;
  }


bool GOContour::add(GOContour *other) {
  if (!other) return false;
  if (kute::isEqual(other->startPoint(), startPoint())) {
     auto& v = other->segments();

     for (auto i = v.rbegin(); i != v.rend(); ++i)
         add(*i);
     delete other;

     return true;
     }
  else if (kute::isEqual(other->startPoint(), endPoint())) {
     auto& v = other->segments();

     for (auto i = v.begin(); i != v.end(); ++i)
         add(*i);
     delete other;

     return true;
     }
  else if (kute::isEqual(other->endPoint(), startPoint())) {
     const auto& v = other->segments();

     for (auto i = v.rbegin(); i != v.rend(); ++i)
         add(*i);
     delete other;

     return true;
     }
  else if (kute::isEqual(other->endPoint(), endPoint())) {
     auto& v = other->segments();

     for (auto i = v.begin(); i != v.end(); ++i)
         add(*i);
     delete other;

     return true;
     }
  return false;
  }


//GraphicObject* GOContour::add(TopoDS_Shape s, double gap) {
//  const TopoDS_Edge e   = TopoDS::Edge(s);
//  GraphicObject*    rv  = nullptr;
//  GraphicObject*    xrv = nullptr;

//  if (!e.IsNull()) {
//     double             param0, param1;
//     Handle(Geom_Curve) c = BRep_Tool::Curve(e, param0, param1);
//     gp_Pnt             p0, p1;

//     p0 = c->Value(param0);
//     p1 = c->Value(param1);

//     if (c->DynamicType() == STANDARD_TYPE(Geom_Line)) {
////        qDebug() << "segment is LINE from" << p0.X() << " / " << p0.Y() << " / " << p0.Z()
////                 << " to "                 << p1.X() << " / " << p1.Y() << " / " << p1.Z();
//        p0.SetZ(center.Z());
//        p1.SetZ(center.Z());
//        rv = new GOLine(p0, p1);
//        }
//     else if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
//        Handle(Geom_Circle) hc = Handle(Geom_Circle)::DownCast(c);
//        gp_Pnt              cc = hc->Circ().Location(); cc.SetZ(center.Z());
//        gp_Circ             circle(gp_Ax2(cc, hc->Circ().Axis().Direction()), hc->Radius());

//        qDebug() << "segment is CIRCLE from" << p0.X() << " / " << p0.Y() << " / " << p0.Z()
//                 << " to "                   << p1.X() << " / " << p1.Y() << " / " << p1.Z();

//        gp_Pnt pn0 = hc->Value(param0);
//        gp_Pnt pn1 = hc->Value(param1);

//        qDebug() << "check CIRCLE points: " << pn0.X() << " / " << pn0.Y() << " / " << pn0.Z()
//                 << " to "                  << pn1.X() << " / " << pn1.Y() << " / " << pn1.Z();

//        if (kute::isEqual(p0, p1)) {    // need to split full circles!
//           double p05 = (param1 - param0) / 2;
//           gp_Pnt pm = hc->Value(p05);

//           pm.SetZ(center.Z());
//           rv  = new GOCircle(hc, param0, p05);
//           xrv = new GOCircle(hc, p05, param1);
//           }
//        else
//           rv = new GOCircle(hc, param0, param1);
//        }
//     else {
//        throw std::domain_error("unsupported Geom-Type!");
//        }
//     while (rv) {
//           if (!segs.size()) {
//              segs.push_back(rv);
//              setStartPoint(rv->startPoint());
//              setEndPoint(rv->endPoint());

//              break; // return nullptr;
//              }
//           else {
//              if (kute::isEqual(endPoint(), rv->startPoint())) {
//                 segs.push_back(rv);
//                 setEndPoint(rv->endPoint());

//                 break; //return nullptr;
//                 }
//              else if (kute::isEqual(endPoint(), rv->endPoint())) {
//                 rv->invert();
//                 segs.push_back(rv);
//                 setEndPoint(rv->endPoint());

//                 break; //return nullptr;
//                 }
//              else {
//                 if (kute::isEqual(startPoint(), rv->startPoint())) {
//                    rv->invert();
//                    segs.insert(segs.begin(), rv);
//                    setStartPoint(rv->startPoint());

//                    break; //return nullptr;
//                    }
//                 else if (kute::isEqual(startPoint(), rv->endPoint())) {
//                    segs.insert(segs.begin(), rv);
//                    setStartPoint(rv->startPoint());

//                    break; //return nullptr;
//                    }
//                 else {
//                    qDebug() << "given segment does not fit on either side of contour. append() rejected!";
//                    }
//                 }
//              }
//           return rv;
//           }
//     return xrv;
//     }
//  return rv;
//  }


gp_Pnt GOContour::centerPoint() const {
  return center;
  }


gp_Pnt GOContour::changeStart2Close(const gp_Pnt &p) {
  gp_Pnt ls = startPoint();
  double d  = p.Distance(ls);
  int    n  = 0;

  // ensure that contour is closed
  if (!kute::isEqual(startPoint(), endPoint())) return p;
  for (int i=1; i < segs.size(); ++i) {
      GraphicObject* go = segs.at(i);
      double ds = p.Distance(go->startPoint());

      if (ds < d) {
         n  = i;
         ls = go->startPoint();
         d  = ds;
         }
      }
  for (int i=0; i < n; ++i) {
      GraphicObject* go = segs.at(i);

      segs.push_back(go);
      }
  for (int i=0; i < n; ++i) segs.erase(segs.begin());

  setStartPoint(segs.at(0)->startPoint());
  setEndPoint(segs.at(segs.size() - 1)->endPoint());

  return endPoint();
  }


bool compContour(GOContour* left, GOContour* right) {
  if (!left || !right) return false;
  return left->a0() < right->a1();
  }


double GOContour::distStart() const {
  return startPoint().Distance(center);
  }


double GOContour::distEnd() const {
  return endPoint().Distance(center);
  }


void GOContour::dump() const {
  qDebug() << "<< ==============>> dump contour segs >>=====================";
  qDebug() << "contour from:" << startPoint().X() << " / " << startPoint().Y()
           << "   to   "      << endPoint().X() << " / " << endPoint().Y()
           << "   with center: " << centerPoint().X() << " / " << centerPoint().Y();
  for (int i=0; i < segs.size(); ++i) {
      GraphicObject* go = segs.at(i);

      go->dump();
      }
  qDebug() << "<< --------------<< dump contour segs <<---------------------";
  }


//TODO: check and ignore closed contours!
GraphicObject& GOContour::extendBy(double length) {
  if (kute::isEqual(startPoint(), endPoint())) return *this;
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

     if (c.IsNull()) {
        qDebug() << "OUPS";
        }
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


gp_Pnt GOContour::midPoint() const {
  if (!segs.size()) return center;
  int n = segs.size() / 2;

  return segs.at(n)->endPoint();
  }


GraphicObject* GOContour::occ2GO(const TopoDS_Edge e, double defZ) {
  if (e.IsNull()) return nullptr;
  GraphicObject*     rv  = nullptr;
  GraphicObject*     xrv = nullptr;
  double             param0, param1;
  Handle(Geom_Curve) c = BRep_Tool::Curve(e, param0, param1);
  gp_Pnt             p0, p1;

  p0 = c->Value(param0);
  p1 = c->Value(param1);

  qDebug() << "occ2GO - curve parameters: " << param0 << " <> " << param1;

  if (c->DynamicType() == STANDARD_TYPE(Geom_Line)) {
     qDebug() << "segment is LINE from" << p0.X() << " / " << p0.Y() << " / " << p0.Z()
              << " to "                 << p1.X() << " / " << p1.Y() << " / " << p1.Z();
     rv = new GOLine(p0, p1);
     }
  else if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
     qDebug() << "segment is CIRCLE from" << p0.X() << " / " << p0.Y() << " / " << p0.Z()
              << " to "                   << p1.X() << " / " << p1.Y() << " / " << p1.Z();
     Handle(Geom_Circle) hc = Handle(Geom_Circle)::DownCast(c);

     //TODO: move splitting to caller!
//     if (kute::isEqual(p0, p1)) {    // need to split full circles!
//        double p05 = (param1 - param0) / 2;
//        gp_Pnt pm = hc->Value(p05);

//        if (!kute::isEqual(defZ, 0)) pm.SetZ(defZ);

//        qDebug() << "splitted full circle at " << pm.X() << " / " << pm.Y() << " / " << pm.Z();

//        rv  = new GOCircle(hc, param0, p05);
//        xrv = new GOCircle(hc, p05, param1);
//        }
//     else
       rv = new GOCircle(hc, param0, param1);
     }
  return rv;
  }


int GOContour::order() const {
  return level;
  }


std::vector<GraphicObject*>& GOContour::segments() {
  return segs;
  }


void GOContour::setContour(TopoDS_Shape contour) {
  Handle(TopTools_HSequenceOfShape) edgePool = new TopTools_HSequenceOfShape;
  TopoDS_Shape                      offWire  = Core().helper3D()->allEdgesWithin(contour, edgePool);
  std::vector<TopoDS_Edge>          segments = Core().helper3D()->allEdgesWithin(offWire);

  if (segments.size()) {
     segs.clear();
     for (auto s : segments) {
         GraphicObject* go = GOContour::occ2GO(s);

         if (go) add(go);
         }
     }
  }


std::vector<GraphicObject*>& GOContour::simplify(double z, bool cw) {
  if (segs.size() > 2) {
     for (int i=0; i < segs.size(); ++i) {
         GraphicObject* o0 = segs.at(i);
         GraphicObject* o1 = segs.at((i+1) < segs.size() ? (i+1) : 0);

         if (kute::isEqual(o0->startPoint(), o0->endPoint())) {
            segs.erase(segs.begin() + i--);
            continue;
            }
         if (o0->type() == GTLine && o1->type() == GTLine) {
            GOLine* l0 = static_cast<GOLine*>(o0);
            GOLine* l1 = static_cast<GOLine*>(o1);

            l0->setZ(z);
            l1->setZ(z);
            double  p2 = l0->p1 + (l1->p1 - l1->p0);

            if (kute::isEqual(l0->curve->Value(p2), l1->endPoint())) {
               l0->extendEnd(l1->p1 - l1->p0);
               if ((i+1) < segs.size()) {
                  segs.erase(segs.begin() + i + 1);
                  }
               else {
                  setEndPoint(o0->endPoint());
                  segs.erase(segs.begin());
                  setStartPoint(segs.at(0)->startPoint());
                  }
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

      Core().shapeFix().SetTolerance(s, kute::MinDelta);
      wireBuilder.Add(TopoDS::Edge(s));
      if (wireBuilder.Error()) break;
      }
  return wireBuilder.Wire();
  }


Handle(AIS_Shape) GOContour::toShape(double z) {
//  return new AIS_Shape(toWire(z));
  BRep_Builder    b;
  TopoDS_Compound c;

  b.MakeCompound(c);
  for (int n=0; n < segs.size(); ++n) {
      auto         go = segs.at(n);
      TopoDS_Shape s  = go->toShape(z)->Shape();

      Core().shapeFix().SetTolerance(s, kute::MinDelta);
      b.Add(c, s);
      }
  return new AIS_Shape(c);
  }


QString GOContour::toString() const {
  QString rv = GraphicObject::toString();

  rv += QString(";%1/%2/%3;%4").arg(center.X(), 0, 'f', 4)
                               .arg(center.Y(), 0, 'f', 4)
                               .arg(center.Z(), 0, 'f', 4)
                               .arg(level);
  for (auto s : segs) {
      rv += "|";
      rv += s->toString();
      }
  return rv;
  }
