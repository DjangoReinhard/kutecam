/* 
 * **************************************************************************
 * 
 *  file:       targetdeflistmodel.cpp
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
#include "targetdeflistmodel.h"
#include "targetdefinition.h"


TargetDefListModel::TargetDefListModel(QObject* parent)
 : QAbstractListModel(parent) {
  }


void TargetDefListModel::append(TargetDefinition* td) {
  int newRow = list.size();

  beginInsertRows(QModelIndex(), newRow, newRow);
  list.push_back(td);
  endInsertRows();
  }


QVariant TargetDefListModel::data(int row, int column, int role) const {
  QModelIndex mi = createIndex(row, column);

  return data(mi, role);
  }


QVariant TargetDefListModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole) return QVariant();
  if (index.row() >= list.size()) return QVariant();
  TargetDefinition* td = list.at(index.row());

  if (!index.column()) return td->toString();
  return QVariant();
  }


TargetDefinition* TargetDefListModel::item(int row) const {
  return list.at(row);
  }


std::vector<TargetDefinition*>& TargetDefListModel::itemList() {
  return list;
  }


bool TargetDefListModel::removeRows(int row, int count, const QModelIndex& parent) {
  int os = list.size();

  beginRemoveRows(parent, row, row);
  list.erase(list.begin() + row);
  endRemoveRows();

  return list.size() < os;
  }


void TargetDefListModel::replaceData(const std::vector<TargetDefinition *>& data) {
  clear();
  beginResetModel();
  list = data;
  endResetModel();
  }


int TargetDefListModel::rowCount(const QModelIndex &parent) const {
  return list.size();
  }


void TargetDefListModel::clear() {
  beginResetModel();
  list.clear();
  endResetModel();
  }

