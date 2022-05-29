/* 
 * **************************************************************************
 * 
 *  file:       geomlistmodel.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    13.2.2022 by Django Reinhard
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
#include "geomnodemodel.h"
#include "core.h"
#include "util3d.h"
#include <BRep_Tool.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <TopoDS.hxx>
#include <QDebug>


GeomNode::GeomNode(GeomType gt)
 : geomType(gt)
 , parentNode(nullptr) {
  }


GeomNode::GeomNode(GeomType gt, const gp_Pnt& p0, const gp_Pnt& p1)
 : geomType(gt)
 , parentNode(nullptr)
 , p0(p0)
 , p1(p1) {
  }


GeomNode::GeomNode(GeomType gt, const gp_Pnt& p0, const gp_Dir& p1)
 : geomType(gt)
 , parentNode(nullptr)
 , p0(p0)
 , p1(p1.X(), p1.Y(), p1.Z()) {
  }


void GeomNode::appendChild(GeomNode* child) {
  child->parentNode = this;
  children.push_back(child);
  }


GeomNode* GeomNode::child(int row) {
  if (row >= children.size()) return nullptr;
  return children.at(row);
  }


int GeomNode::childCount() const {
  return children.size();
  }


int GeomNode::childNumber() const {
  if (parentNode)
     return parentNode->children.indexOf(const_cast<GeomNode*>(this));
  return 0;
  }


void GeomNode::clear() {
  geomType = Geom_INVALID;
  children.clear();
  }


int GeomNode::columnCount() const {
  return 7;
  }


QVariant GeomNode::data(int column) const {
  switch (column) {
    case 0:  return type2String();
    case 1:  return QString("%1").arg(p0.X(), 0, 'f', 3);
    case 2:  return QString("%1").arg(p0.Y(), 0, 'f', 3);
    case 3:  return QString("%1").arg(p0.Z(), 0, 'f', 3);
    case 4:  return QString("%1").arg(p1.X(), 0, 'f', 3);
    case 5:  return QString("%1").arg(p1.Y(), 0, 'f', 3);
    case 6:  return QString("%1").arg(p1.Z(), 0, 'f', 3);
    default: break;
    }
  return QVariant();
  }


GeomNode* GeomNode::parent() const {
  return parentNode;
  }


QString GeomNode::type2String() const {
  switch (geomType) {
    case Geom_FACE:    return tr("Face");
    case Geom_PLANE:   return tr("Plane");
    case Geom_LINE:    return tr("Line");
    case Geom_ELLIPSE: return tr("Ellipse");
    case Geom_CIRCLE:  return tr("Circle");
    case Geom_CURVE:   return tr("Curve");
    default:           return tr("Type");
    }
  return QString();
  }


GeomNodeModel::GeomNodeModel(QObject *parent)
 : QAbstractItemModel(parent)
 , root(new GeomNode(Geom_INVALID, gp_Pnt(0, 0, 0), gp_Pnt(1, 1, 1))) {
  }


GeomNodeModel::~GeomNodeModel() {
  delete root;
  }


int GeomNodeModel::columnCount(const QModelIndex& parent) const {
  if (parent.isValid())
     return static_cast<GeomNode*>(parent.internalPointer())->columnCount();
  return root->columnCount();
  }


QVariant GeomNodeModel::data(int row, int column, int role) const {
  QModelIndex mi = createIndex(row, column);

  return data(mi, role);
  }


QVariant GeomNodeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();
  if (index.column() && role == Qt::TextAlignmentRole) return int(Qt::AlignRight | Qt::AlignVCenter);
  if (role != Qt::DisplayRole) return QVariant();
  GeomNode* item = static_cast<GeomNode*>(index.internalPointer());

  return item->data(index.column());
  }


void GeomNodeModel::exploreCurve(GeomNode* p, Handle(Geom_Curve) c, double first, double last) {
  if (c->DynamicType() == STANDARD_TYPE(Geom_Line)) {
     Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(c);
     gp_Pnt p0 = c->Value(first);
     gp_Pnt p1 = c->Value(last);

     qDebug() << "edge is a straight line from"
              << p0.X() << " / "
              << p0.Y() << " / "
              << p0.Z() << "   to   "
              << p1.X() << " / "
              << p1.Y() << " / "
              << p1.Z();
     p->appendChild(new GeomNode(Geom_LINE, p0, p1));
     }
  else if (c->DynamicType() == STANDARD_TYPE(Geom_Ellipse)) {
     Handle(Geom_Ellipse) ellipse = Handle(Geom_Ellipse)::DownCast(c);
     //     qDebug() << "edge is an arc (ellipse)";
     // parameter: gp_Pnt loc  = c->Location();
     //            gpAx1  axis = c->Axis();
     //            double rMax = c->MajorRadius();
     //            double rMin = c->MinorRadius();
     qDebug() << "edge is Ellipse with center at"
              << ellipse->Location().X() << " / "
              << ellipse->Location().Y() << " / "
              << ellipse->Location().Z()
              << "   and R(major): " << ellipse->MajorRadius()
              << "   and R(minor): " << ellipse->MinorRadius();
     p->appendChild(new GeomNode(Geom_ELLIPSE, {ellipse->MajorRadius(), ellipse->MinorRadius(), 0}, ellipse->Location()));
     }
  else if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
     Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
     const gp_Pnt&       pos    = circle->Position().Location();
     const gp_Dir&       dir    = circle->Position().Direction();
     double              radius = circle->Radius();

     qDebug() << "edge is a circle with center at"
              << pos.X() << " / "
              << pos.Y() << " / "
              << pos.Z() << "\t and radius:" << radius;
     p->appendChild(new GeomNode(Geom_CIRCLE, {radius, 0, dir.Z()}, pos));
     }
  else {
     qDebug() << "edge is unknown curve (" << c->DynamicType()->Name() << ")";
     }
  }


void GeomNodeModel::exploreEdge(GeomNode* p, TopoDS_Edge e) {
  if (BRep_Tool::IsGeometric(e)) {
     double             first, last;
     Handle(Geom_Curve) c = BRep_Tool::Curve(e, first, last);

     exploreCurve(p, c, first, last);
     }
  else {
     qDebug() << "OUPS - edge from selected face is NOT geometric?!?";
     }
  }


void GeomNodeModel::exploreFace(GeomNode* p, TopoDS_Face f) {
  std::vector<TopoDS_Edge> edges = Core().helper3D()->allEdgesWithin(f);

  for (auto e : edges) {
      exploreEdge(p, e);
      }
  }


GeomNode* GeomNodeModel::getItem(const QModelIndex &index) const {
  if (index.isValid()) {
     GeomNode* item = static_cast<GeomNode*>(index.internalPointer());

     if (item) return item;
     }
  return root->child(0);
  }


QVariant GeomNodeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
     return root->data(section);
  return QVariant();
  }


QModelIndex GeomNodeModel::index(int row, int column, const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent)) return QModelIndex();
  GeomNode* parentItem;

  if (!parent.isValid()) parentItem = root;
  else                   parentItem = static_cast<GeomNode*>(parent.internalPointer());
  GeomNode* childItem  = parentItem->child(row);

  if (childItem) return createIndex(row, column, childItem);
  return QModelIndex();
  }


QModelIndex GeomNodeModel::parent(const QModelIndex &index) const {
  if (!index.isValid()) return QModelIndex();
  GeomNode* childItem  = getItem(index);
  GeomNode* parentItem = childItem ? childItem->parent() : nullptr;

  if (parentItem == root || !parentItem)
     return QModelIndex();
  return createIndex(parentItem->childNumber(), 0, parentItem);
  }


int GeomNodeModel::rowCount(const QModelIndex &parent) const {
  GeomNode* parentItem;

  if (parent.column() > 0) return 0;
  if (!parent.isValid()) parentItem = root;
  else                   parentItem = static_cast<GeomNode*>(parent.internalPointer());
  return parentItem->childCount();
  }


void GeomNodeModel::clear() {
  beginResetModel();
  root->clear();
  endResetModel();
  }


void GeomNodeModel::replaceData(const std::vector<TopoDS_Shape>& selection) {
  beginResetModel();
  root->clear();
  for (auto s : selection) {
      if (s.ShapeType() == TopAbs_FACE) {
         qDebug() << "selection is face ...";

         Handle(Geom_Surface) selectedFace = BRep_Tool::Surface(TopoDS::Face(s));
         GeomNode* node = nullptr;

         if (selectedFace->IsKind(STANDARD_TYPE(Geom_Plane))) {
            GeomAdaptor_Surface selectedSurface(selectedFace);
            gp_Pln              pln = selectedSurface.Plane();
            gp_Pnt              pos = Core().helper3D()->deburr(pln.Location());
            gp_Dir              dir = Core().helper3D()->deburr(pln.Axis().Direction());

            node = new GeomNode(Geom_PLANE, pos, dir);
            }
         else {
            node = new GeomNode(Geom_FACE);
            }
         root->appendChild(node);

         exploreFace(node, TopoDS::Face(s));
         }
      else if (s.ShapeType() == TopAbs_EDGE) {
         qDebug() << "selection is edge ...";

         exploreEdge(root, TopoDS::Edge(s));
         }
      else {
         qDebug() << "selection is unknown: " << s.ShapeType();
         }
      }
  endResetModel();
  }
