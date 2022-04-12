/* 
 * **************************************************************************
 * 
 *  file:       infopage.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    11.4.2022 by Django Reinhard
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
#include "infopage.h"
#include "ui_info.h"
#include "util3d.h"
#include "core.h"
#include "geomlistmodel.h"
#include "shapelistmodel.h"
#include "mainwindow.h"
#include "occtviewer.h"
#include "operation.h"
#include "projectfile.h"
#include "work.h"
#include "kuteCAM.h"
#include "polysegment.h"

#include <QListView>
#include <QListWidget>
#include <QSplitter>
#include <QDebug>

#include <BOPAlgo_Tools.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepOffsetAPI_MakeOffsetShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <gp_Quaternion.hxx>


InfoPage::InfoPage(GeomListModel* model, QWidget *parent)
 : ApplicationWindow(parent)
 , ui(new Ui::InfoPage)
 , model(model)
 , listGeom(new QListView)
 , listEdges(new QListWidget) {
  ui->setupUi(this);
  QSplitter* sp = new QSplitter(Qt::Vertical);

  listGeom->setModel(model);
  listGeom->setSelectionMode(QAbstractItemView::ExtendedSelection);
  sp->addWidget(listGeom);
  sp->addWidget(listEdges);
  ui->widget->setLayout(new QVBoxLayout);
  ui->widget->layout()->addWidget(sp);
  ui->dsCut->setValue(11);
  connect(ui->spA,   &QDoubleSpinBox::valueChanged, this, &InfoPage::rotate);
  connect(ui->spB,   &QDoubleSpinBox::valueChanged, this, &InfoPage::rotate);
  connect(ui->spC,   &QDoubleSpinBox::valueChanged, this, &InfoPage::rotate);
  connect(ui->dsCut, &QDoubleSpinBox::valueChanged, this, &InfoPage::updateCut);
  connect(Core().view3D(), &OcctQtViewer::shapeSelected,  this, &InfoPage::shapeSelected);
  connect(listGeom->selectionModel(), &QItemSelectionModel::selectionChanged, this, &InfoPage::geomSelected);
  connect(this, &InfoPage::raiseMessage, Core().mainWin(), &MainWindow::setStatusMessage);
  }


void InfoPage::exploreCurve(Handle(Geom_Curve) c, double first, double last) {
  if (c->DynamicType() == STANDARD_TYPE(Geom_Line)) {
     Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(c);
     const gp_Ax1&     pos  = line->Position();

//     qDebug() << "edge is a straight line"
//              << pos.Location().X() << "/"
//              << pos.Location().Y() << "/"
//              << pos.Location().Z() << "\tdirection:"
//              << pos.Direction().X() << "/"
//              << pos.Direction().Y() << "/"
//              << pos.Direction().Z();
     listEdges->addItem(QString("Pos: %1/%2/%3 - Dir: %4/%5/%6")
                               .arg(pos.Location().X(), 0, 'f', 3)
                               .arg(pos.Location().Y(), 0, 'f', 3)
                               .arg(pos.Location().Z(), 0, 'f', 3)
                               .arg(pos.Direction().X(), 0, 'f', 3)
                               .arg(pos.Direction().Y(), 0, 'f', 3)
                               .arg(pos.Direction().Z(), 0, 'f', 3));
     Core().view3D()->genLine(line, first, last);
     }
  else if (c->DynamicType() == STANDARD_TYPE(Geom_BSplineCurve)) {
     qDebug() << "edge is a bspline curve";
     }
  else if (c->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve)) {
     Handle(Geom_TrimmedCurve) trimmed = Handle(Geom_TrimmedCurve)::DownCast(c);
     Handle(Geom_Curve)        base    = trimmed->BasisCurve();
     gp_Pnt                    start   = trimmed->StartPoint();
     gp_Pnt                    end     = trimmed->EndPoint();

     if (base->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
        Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(base);
        const gp_Pnt&       pos    = circle->Position().Location();
        const gp_Dir&       dir    = circle->Position().Direction();
        double              radius = circle->Radius();

//        qDebug() << "edge is an arc with center at"
//                 << pos.X() << "/"
//                 << pos.Y() << "/"
//                 << pos.Z() << "\t direction:"
//                 << dir.X() << "/"
//                 << dir.Y() << "/"
//                 << dir.Z()
//                 << "\t and radius:" << radius
//                 << "\t and start:"
//                 << start.X() << "/"
//                 << start.Y() << "/"
//                 << start.Z() << "\t and end:"
//                 << end.X() << "/"
//                 << end.Y() << "/"
//                 << end.Z();
        listEdges->addItem(QString("Arc - R:%1 - C: %2/%3/%4 - Dir: %5/%6/%7")
                                  .arg(radius, 0, 'f', 3)
                                  .arg(pos.X(), 0, 'f', 3)
                                  .arg(pos.Y(), 0, 'f', 3)
                                  .arg(pos.Z(), 0, 'f', 3)
                                  .arg(dir.X(), 0, 'f', 3)
                                  .arg(dir.Y(), 0, 'f', 3)
                                  .arg(dir.Z(), 0, 'f', 3));
        listEdges->addItem(QString("Arc from: %1/%2/%3 - to: %4/%5/%6")
                                  .arg(start.X(), 0, 'f', 3)
                                  .arg(start.Y(), 0, 'f', 3)
                                  .arg(start.Z(), 0, 'f', 3)
                                  .arg(end.X(), 0, 'f', 3)
                                  .arg(end.Y(), 0, 'f', 3)
                                  .arg(end.Z(), 0, 'f', 3));
        }
     else {
//        qDebug() << "trimmed curve is not based on circle! start:"
//                 << start.X() << "/"
//                 << start.Y() << "/"
//                 << start.Z() << "\t and end:"
//                 << end.X() << "/"
//                 << end.Y() << "/"
//                 << end.Z();
        listEdges->addItem(QString("NA from: %1/%2/%3 - to: %4/%5/%6")
                                  .arg(start.X(), 0, 'f', 3)
                                  .arg(start.Y(), 0, 'f', 3)
                                  .arg(start.Z(), 0, 'f', 3)
                                  .arg(end.X(), 0, 'f', 3)
                                  .arg(end.Y(), 0, 'f', 3)
                                  .arg(end.Z(), 0, 'f', 3));
        }
     }
  else if (c->DynamicType() == STANDARD_TYPE(Geom_Ellipse)) {
     Handle(Geom_Ellipse) ellipse = Handle(Geom_Ellipse)::DownCast(c);

//     qDebug() << "edge is an arc (ellipse)";
     // parameter: gp_Pnt loc  = c->Location();
     //            gpAx1  axis = c->Axis();
     //            double rMax = c->MajorRadius();
     //            double rMin = c->MinorRadius();
     Core().view3D()->genEllipse(ellipse, first, last);
     }
  else if (c->DynamicType() == STANDARD_TYPE(Geom_BoundedCurve)) {
     Handle(Geom_BoundedCurve) bc    = Handle(Geom_BoundedCurve)::DownCast(c);
     gp_Pnt                    start = bc->StartPoint();
     gp_Pnt                    end   = bc->EndPoint();

//     qDebug() << "edge is an arc (bounded curve)";
     // parameter gp_Pnt start = c->StartPoint();
     //           gp_Pnt end   = c->EndPoint();
     listEdges->addItem(QString("Bounded Curve from: %1/%2/%3 - to: %4/%5/%6")
                               .arg(start.X(), 0, 'f', 3)
                               .arg(start.Y(), 0, 'f', 3)
                               .arg(start.Z(), 0, 'f', 3)
                               .arg(end.X(), 0, 'f', 3)
                               .arg(end.Y(), 0, 'f', 3)
                               .arg(end.Z(), 0, 'f', 3));
     }
  else if (c->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
     Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(c);
     const gp_Pnt&       pos    = circle->Position().Location();
     const gp_Dir&       dir    = circle->Position().Direction();
     double              radius = circle->Radius();

//     qDebug() << "edge is a circle with center at"
//              << pos.X() << "/"
//              << pos.Y() << "/"
//              << pos.Z() << "\t direction:"
//              << dir.X() << "/"
//              << dir.Y() << "/"
//              << dir.Z()
//              << "\t and radius:" << radius;
     listEdges->addItem(QString("R: %1 - C: %2/%3/%4 - Dir: %5/%6/%7")
                               .arg(radius, 0, 'f', 3)
                               .arg(pos.X(), 0, 'f', 3)
                               .arg(pos.Y(), 0, 'f', 3)
                               .arg(pos.Z(), 0, 'f', 3)
                               .arg(dir.X(), 0, 'f', 3)
                               .arg(dir.Y(), 0, 'f', 3)
                               .arg(dir.Z(), 0, 'f', 3));
     Core().view3D()->genCircle(circle, first, last);
     }
  else {
     qDebug() << "edge is unknown curve (" << c->DynamicType()->Name() << ")";
     }
//  qDebug() << "\tand has param range" << first << " - " << last;
  listEdges->addItem(QString("Param-Range: %1 - %2 (%3 - %4)")
                            .arg(first, 0, 'f', 3)
                            .arg(last, 0, 'f', 3)
                            .arg(rad2deg(first), 0, 'f', 1)
                            .arg(rad2deg(last), 0, 'f', 1));
  }


void InfoPage::exploreEdge(const TopoDS_Shape &shape) {
  const TopoDS_Edge& edge = static_cast<const TopoDS_Edge&>(shape);

  if (BRep_Tool::IsGeometric(edge)) {
//     qDebug() << "edge is geometric";
     double first, last;
     Handle(Geom_Curve) c = BRep_Tool::Curve(edge, first, last);

     exploreCurve(c, first, last);
     }
  else {
     qDebug() << "edge is NOT geometric";
     }
  }


cavc::Polyline<double> InfoPage::exploreShape(Handle(AIS_Shape) s) {
  std::vector<TopoDS_Edge> edges    = Core().helper3D()->allEdgesWithin(s->Shape());
  cavc::Polyline<double>   polyLine = Core().helper3D()->toPolyline(edges);

  Core().helper3D()->dumpPolyline(polyLine);

  return polyLine;
  }


QListView* InfoPage::geomList() {
  return listGeom;
  }


void InfoPage::geomSelected(const QItemSelection &selected, const QItemSelection &deselected) {
  QModelIndexList il = selected.indexes();
  int             mx = il.count();

  listEdges->clear();
  for (int i = 0; i < mx; ++i) {
      QModelIndex              mi    = il.at(i);
      Handle(Geom_Curve)       c     = model->item(mi.row());

      exploreCurve(c, model->data(mi.row(), 1).toDouble(), model->data(mi.row(), 2).toDouble());
      }
  }


QListWidget* InfoPage::edgeList() {
  return listEdges;
  }


void InfoPage::rotate() {
  double dA = ui->spA->value();
  double dB = ui->spB->value();
  double dC = ui->spC->value();

  Core().view3D()->rotate(deg2rad(dA), deg2rad(dB), deg2rad(dC));
  }


void InfoPage::shapeSelected(const TopoDS_Shape& shape) {
  qDebug() << "InfoPage::shapeSelected() ...";
#ifdef REDNOSE
  std::vector<TopoDS_Edge> edges = Core().helper3D()->allEdgesWithin(shape);

//  for (auto& e : edges) exploreEdge(e);
  Handle(AIS_Shape) curWP    = Core().helper3D()->fixRotation(shape
                                                 , ui->spA->value()
                                                 , ui->spB->value()
                                                 , ui->spC->value());
  std::set<gp_Pnt>  vertices = Core().helper3D()->allVertexCoordinatesWithin(curWP->Shape());
  gp_Pnt top(0, 0, 0);

  for (auto v : vertices)
      if (v.Z() > top.Z()) top = v;

  qDebug() << "top of workpiece:" << top.X() << "/" << top.Y() << "/" << top.Z();

  if (shape.ShapeType() == TopAbs_FACE) {
     Handle(Geom_Surface) selectedFace = BRep_Tool::Surface(TopoDS::Face(shape));

     if (selectedFace->IsKind(STANDARD_TYPE(Geom_Plane))) {
        double       offset = 1;
        TopoDS_Shape s;

        for (int i=0; i < 1; ++i, offset += 5) {
            BRepOffsetAPI_MakeOffsetShape mos;

            mos.PerformBySimple(shape, offset);
            s = mos.Shape();
            BRepAdaptor_Surface tmp(TopoDS::Face(s), false);
            GeomAdaptor_Surface surface = tmp.Surface();
            BRepBuilderAPI_MakeFace mf(surface.Plane());
            TopoDS_Shape res = Core().helper3D()->intersect(curWP->Shape(), mf.Shape());

            vertices = Core().helper3D()->allVertexCoordinatesWithin(res);
            for (auto v : vertices) {
                listEdges->addItem(QString("%1/%2/%3").arg(v.X(), 0, 'f', 3)
                                                      .arg(v.Y(), 0, 'f', 3)
                                                      .arg(v.Z(), 0, 'f', 3));
                }
            listEdges->addItem(" ");
            Handle(AIS_Shape) as = new AIS_Shape(res);
            as->SetColor(Quantity_NOC_RED);
            Core().view3D()->showShape(as);
            }
        }
     }
  else {
     vertices = Core().helper3D()->allVertexCoordinatesWithin(shape);

     for (auto v : vertices) {
         listEdges->addItem(QString("%1/%2/%3").arg(v.X(), 0, 'f', 3)
                                               .arg(v.Y(), 0, 'f', 3)
                                               .arg(v.Z(), 0, 'f', 3));
         }
     listEdges->addItem(" ");
     }
#else
  std::vector<TopoDS_Edge> edges = Core().helper3D()->allEdgesWithin(shape);

  for (auto e : edges) {
      if (BRep_Tool::IsGeometric(e)) {
         double             first, last;
         Handle(Geom_Curve) c = BRep_Tool::Curve(e, first, last);

         model->append(c, first, last);
         }
      else {
         qDebug() << "OUPS - edge from selected face is NOT geometric?!?";
         }
      }
#endif
  }


void InfoPage::toolPathChanged(Operation* op) {
//  model->setData(op->toolPaths);
  qDebug() << "InfoPage::toolPathChanged( ... ) - ?!?";
  }


void InfoPage::updateCut() {
  gp_Pln                  p({0, 0, ui->dsCut->value()}, {0, 0, 1});
  BRepBuilderAPI_MakeFace mf(p, -500, 500, -500, 500);
  Work*                   work  = Core().workData();
  Handle(AIS_Shape)       model = Core().helper3D()->fixRotation(work->model->Shape()
                                                    , ui->spA->value()
                                                    , ui->spB->value()
                                                    , ui->spC->value());
  Handle(AIS_Shape)       curWP = Core().helper3D()->fixRotation(work->workPiece->Shape()
                                                    , ui->spA->value()
                                                    , ui->spB->value()
                                                    , ui->spC->value());

  if (!work->modCut.IsNull()) Core().view3D()->removeShape(work->modCut);
  if (!work->wpCut.IsNull())  Core().view3D()->removeShape(work->wpCut);
  work->wpCut = new AIS_Shape(Core().helper3D()->intersect(mf.Shape(), curWP->Shape()));
  work->wpCut->SetColor(Quantity_NOC_CYAN);
  work->modCut = new AIS_Shape(Core().helper3D()->intersect(mf.Shape(), model->Shape()));
  work->modCut->SetColor(Quantity_NOC_PURPLE);

  shapeSelected(work->modCut->Shape());

#ifdef REDNOSE
  cavc::Polyline<double> outerPoly;
  double                 offset      = 0.5;
  cavc::Polyline<double> polyLine    = exploreShape(work->modCut);
  auto                   prunedPline = pruneSingularities(polyLine, cavc::utils::realPrecision<double>());
  cavc::Polyline<double> rawOffsetPline;
  cavc::Polyline<double> dualRawOffsetPline;

  outerPoly.addVertex(-90, 0, 1);
  outerPoly.addVertex(90, 0, 1);
  outerPoly.isClosed() = true;
  Core().helper3D()->dumpPolyline(prunedPline);
  for (int i=0; i < 6; ++i, offset += 5) {
      rawOffsetPline = cavc::internal::createRawOffsetPline(prunedPline, offset);
      Core().helper3D()->dumpPolyline(rawOffsetPline);
      if (dualRawOffsetPline.size() == 0)
         dualRawOffsetPline = cavc::internal::createRawOffsetPline(prunedPline, -offset);
      auto slices = cavc::internal::dualSliceAtIntersectsForOffset(prunedPline
                                                                 , rawOffsetPline
                                                                 , dualRawOffsetPline
                                                                 , offset);
      auto stichedPlines = stitchOffsetSlicesTogether(slices
                                                    , rawOffsetPline.isClosed()
                                                    , rawOffsetPline.size() - 1);
      for (const auto& pline : stichedPlines) {
          Core().view3D()->showPolyLine(pline, ui->dsCut->value() - 10, Quantity_NOC_YELLOW);
          auto combineResult = combinePolylines(outerPoly, pline, cavc::PlineCombineMode::Intersect);

          for (const auto& pl : combineResult.remaining) {
              Core().view3D()->showPolyLine(pl, ui->dsCut->value(), Quantity_NOC_PURPLE);
              }
          }
      }
  Core().view3D()->showPolyLine(polyLine, ui->dsCut->value(), Quantity_NOC_DARKGREEN);
#endif
  Core().view3D()->showShape(work->modCut);
  Core().view3D()->showShape(work->wpCut);
  Core().view3D()->refresh();
  }
