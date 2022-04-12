/* 
 * **************************************************************************
 * 
 *  file:       shapelistmodel.cpp
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
#include "shapelistmodel.h"
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS.hxx>


ShapeListModel::ShapeListModel(QObject *parent)
 : QAbstractListModel(parent) {
  }


void ShapeListModel::append(opencascade::handle<AIS_Shape> shape) {
  int        newRow = list.size();

  beginInsertRows(QModelIndex(), newRow, newRow);
  list.push_back(shape);
  endInsertRows();
  }


QVariant ShapeListModel::data(int row, int column, int role) const {
  QModelIndex mi = createIndex(row, column);

  return data(mi, role);
  }


QVariant ShapeListModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole) return QVariant();
  if (index.row() >= list.size()) return QVariant();
  Handle(AIS_Shape)     cd = list.at(index.row());
  TopoDS_Shape          cs = cd->Shape();

  if (!index.column()) {
     return QString("Shape #%1").arg(index.row());
     }
  return QVariant();
  }


void ShapeListModel::setData(const std::vector<Handle(AIS_Shape)>& list) {
  beginResetModel();
  this->list = list;
  endResetModel();
  }


void ShapeListModel::setData(const QVector<Handle(AIS_Shape)>& list) {
  beginResetModel();
  this->list.clear();
  for (Handle(AIS_Shape) s: list) {
      this->list.push_back(s);
      }
  endResetModel();
  }


Handle(AIS_Shape) ShapeListModel::item(int row) const {
  return list.at(row);
  }


int ShapeListModel::rowCount(const QModelIndex &parent) const {
  return list.size();
  }


void ShapeListModel::clear() {
  beginResetModel();
  list.clear();
  endResetModel();
  }

