/* 
 * **************************************************************************
 * 
 *  file:       shapelistmodel.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    26.3.2022 by Django Reinhard
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
#ifndef SHAPELISTMODEL_H
#define SHAPELISTMODEL_H
#include <QAbstractListModel>
#include <QVector>
#include <AIS_Shape.hxx>


class ShapeListModel : public QAbstractListModel
{
public:
  explicit ShapeListModel(QObject *parent = nullptr);

  void               append(Handle(AIS_Shape) shape);
  void               clear();
  QVariant           data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant           data(int row, int column, int role = Qt::DisplayRole) const;
  Handle(AIS_Shape)  item(int row = 0) const;
  int                rowCount(const QModelIndex &parent = QModelIndex()) const override;
  void               setData(const std::vector<Handle(AIS_Shape)>& list);
  void               setData(const QVector<Handle(AIS_Shape)>& list);

private:
  std::vector<Handle(AIS_Shape)> list;
  };
#endif // SHAPELISTMODEL_H
