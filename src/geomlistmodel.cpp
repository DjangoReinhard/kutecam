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
#include "geomlistmodel.h"
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_BSplineCurve.hxx>


GeomListModel::GeomListModel(QObject *parent)
 : QAbstractListModel(parent) {
  }


void GeomListModel::append(opencascade::handle<Geom_Curve> curve, double first, double last) {
  int        newRow = list.size();
  CurveData* cd     = new CurveData(curve, first, last);

  if (list.contains(cd)) return;
  beginInsertRows(QModelIndex(), newRow, newRow);
  list.append(cd);
  endInsertRows();
  }


QVariant GeomListModel::data(int row, int column, int role) const {
  QModelIndex mi = createIndex(row, column);

  return data(mi, role);
  }


QVariant GeomListModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole) return QVariant();
  if (index.row() >= list.size()) return QVariant();
  CurveData* cd = list.at(index.row());

  if (index.column() == 0) {
     if (cd->curve->DynamicType() == STANDARD_TYPE(Geom_Line)) {
        return "Geom_Line";
        }
     else if (cd->curve->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
        return "Geom_Circle";
        }
     else if (cd->curve->DynamicType() == STANDARD_TYPE(Geom_BSplineCurve)) {
        return "Geom_BSplineCurve";
        }
    else if (cd->curve->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve)) {
        return "Geom_TrimmedCurve";
        }
     else if (cd->curve->DynamicType() == STANDARD_TYPE(Geom_Ellipse)) {
        return "Geom_Ellipse";
        }
     else if (cd->curve->DynamicType() == STANDARD_TYPE(Geom_BoundedCurve)) {
        return "Geom_BoundedCurve";
        }
     return "unknown Curve";
     }
  else if (index.column() == 1) return cd->first;
  else if (index.column() == 2) return cd->last;
  return QVariant();
  }


Handle(Geom_Curve) GeomListModel::item(int row) const {
  CurveData*         cd = list.at(row);
  Handle(Geom_Curve) rv;

  if (cd) return list.at(row)->curve;
  return rv;
  }


int GeomListModel::rowCount(const QModelIndex &parent) const {
  return list.size();
  }


void GeomListModel::clear() {
  beginResetModel();
  list.clear();
  endResetModel();
  }


bool GeomListModel::CurveData::operator==(const CurveData &other) {
  if (&other == this) return true;
  return curve == other.curve && first == other.first && last == other.last;
  }
