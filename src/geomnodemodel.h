/* 
 * **************************************************************************
 * 
 *  file:       geomlistmodel.h
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
#ifndef GEOMLISTMODEL_H
#define GEOMLISTMODEL_H
#include <QAbstractItemModel>
#include <Geom_Curve.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>


enum GeomType
{
  Geom_FACE
, Geom_PLANE
, Geom_LINE
, Geom_ELLIPSE
, Geom_CIRCLE
, Geom_CURVE
, Geom_INVALID
  };


class GeomNode : public QObject
{
  Q_OBJECT
public:
  explicit GeomNode(GeomType gt);
  explicit GeomNode(GeomType gt, const gp_Pnt& p0, const gp_Pnt& p1);
  explicit GeomNode(GeomType gt, const gp_Pnt& p0, const gp_Dir& d);

  void     appendChild(GeomNode* child);

  GeomNode* child(int row);
  int       childCount() const;
  int       childNumber() const;
  void      clear();
  int       columnCount() const;
  QVariant  data(int column) const;
  GeomNode* parent() const;
  QString   type2String() const;

private:
  QVector<GeomNode*> children;
  GeomType           geomType;
  GeomNode*          parentNode;
  gp_Pnt             p0;
  gp_Pnt             p1;
  };


class GeomNodeModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit GeomNodeModel(QObject *parent = nullptr);
  virtual ~GeomNodeModel();

  void          clear();
  int           columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant      data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant      data(int row, int column, int role = Qt::DisplayRole) const;
  GeomNode*     getItem(const QModelIndex &index) const;
  QVariant      headerData(int section, Qt::Orientation orientation, int role) const override;
  QModelIndex   index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex   parent(const QModelIndex& index) const override;
  //  Handle(Geom_Curve) item(int row = 0) const;
  int           rowCount(const QModelIndex &parent = QModelIndex()) const override;
  void          replaceData(const std::vector<TopoDS_Shape>& selection);

protected:
  void exploreCurve(GeomNode* parent, Handle(Geom_Curve) c, double first, double last);
  void exploreEdge(GeomNode* parent, TopoDS_Edge e);
  void exploreFace(GeomNode* parent, TopoDS_Face f);

private:
  GeomNode* root;
  };
#endif // GEOMLISTMODEL_H
