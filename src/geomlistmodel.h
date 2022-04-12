/* 
 * **************************************************************************
 * 
 *  file:       geomlistmodel.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    13.2.2022 by Django Reinhard
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
#ifndef GEOMLISTMODEL_H
#define GEOMLISTMODEL_H
#include <QAbstractListModel>
#include <QVector>
#include <Geom_Curve.hxx>



class GeomListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  explicit GeomListModel(QObject *parent = nullptr);

  void               append(Handle(Geom_Curve) curve, double first, double last);
  QVariant           data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant           data(int row, int column, int role = Qt::DisplayRole) const;
  Handle(Geom_Curve) item(int row = 0) const;
  int                rowCount(const QModelIndex &parent = QModelIndex()) const override;
  void               clear();

private:
  struct CurveData
  {
  CurveData(Handle(Geom_Curve) c, double f, double l)
   : curve(c)
   , first(f)
   , last(l) {
    }
  bool operator==(const CurveData& other);
  Handle(Geom_Curve) curve;
  double             first;
  double             last;
    };
  QVector<CurveData*> list;
  };
#endif // GEOMLISTMODEL_H
