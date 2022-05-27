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
#include <TopoDS_Shape.hxx>


class GeomNode : public QObject
{
  Q_OBJECT
public:
  explicit GeomNode(TopoDS_Shape s);


  };

class GeomNodeModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit GeomNodeModel(QObject *parent = nullptr);

  void          append(TopoDS_Shape shape);
  int           columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant      data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant      data(int row, int column, int role = Qt::DisplayRole) const;
  QModelIndex   index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex   parent(const QModelIndex& index) const override;
  //  Handle(Geom_Curve) item(int row = 0) const;
  int           rowCount(const QModelIndex &parent = QModelIndex()) const override;
  void          clear();

private:
  std::vector<GeomNode*> nodes;
  };
#endif // GEOMLISTMODEL_H
