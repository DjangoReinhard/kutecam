/* 
 * **************************************************************************
 * 
 *  file:       infopage.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    6.4.2022 by Django Reinhard
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
#ifndef INFOPAGE_H
#define INFOPAGE_H
#include "applicationwindow.h"
#include <Geom_Curve.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <AIS_Shape.hxx>
#include <cavc/polyline.hpp>
QT_BEGIN_NAMESPACE
namespace Ui {
class InfoPage;
}
QT_END_NAMESPACE
class GeomListModel;
class Operation;
class ProjectFile;
class ShapeListModel;
class QDoubleSpinBox;
class QListView;
class QListWidget;
class QSpinBox;
class QItemSelection;
class Util3D;
class Work;


class InfoPage : public ApplicationWindow
{
  Q_OBJECT
public:
  explicit InfoPage(GeomListModel* model, QWidget *parent = nullptr);
//  explicit InfoPage(ShapeListModel* model, QWidget *parent = nullptr);

  QListView*               geomList();
  QListWidget*             edgeList();
//  void                     dumpPolyline(const cavc::Polyline<double>& pl);
  void                     exploreCurve(Handle(Geom_Curve) curve, double first, double last);
  void                     exploreEdge(const TopoDS_Shape& shape);
  cavc::Polyline<double>   exploreShape(Handle(AIS_Shape) s);
//  cavc::Polyline<double>   createPolyline(const std::vector<TopoDS_Edge>& edges);
//  TopoDS_Edge              flipCurve(Handle(Geom_Curve) c, double param0, double param1);
  void                     geomSelected(const QItemSelection& selected, const QItemSelection& deselected);
  void                     rotate();
//  std::vector<TopoDS_Edge> sortEdges(std::vector<TopoDS_Edge>& edges);

signals:
  void raiseMessage(const QString& msg);

public slots:
  void toolPathChanged(Operation* op);

protected slots:
  void shapeSelected(const TopoDS_Shape& shape);
  void updateCut();

private:
  Ui::InfoPage*    ui;
  GeomListModel*   model;
//  ShapeListModel*  model;
  QListView*       listGeom;
  QListWidget*     listEdges;
  };
#endif // INFOPAGE_H
