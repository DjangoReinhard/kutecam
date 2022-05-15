/* 
 * **************************************************************************
 * 
 *  file:       stringlistmodel.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    4.4.2022 by Django Reinhard
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
#include "stringlistmodel.h"


StringListModel::StringListModel(const QStringList& sl, QObject *parent)
 : QAbstractItemModel(parent) {
  for (const QString& s : sl)
      sList.append(s);
  }


void StringListModel::add(const QString& item) {
  int row = sList.count();

  beginInsertRows(QModelIndex(), row, row);
  sList.append(item);
  endInsertRows();
  }


int StringListModel::columnCount(const QModelIndex& parent) const {
  return 1;
  }


QVariant StringListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();
  else if (role == Qt::DisplayRole) {
    const QString& s = sList.at(index.row());

    return s;
    }
  return QVariant();
  }


QVariant StringListModel::headerData(int section, Qt::Orientation orientation, int role) const {
  return tr("Items");
  }


QModelIndex StringListModel::index(int row, int column, const QModelIndex& parent) const {
  QModelIndex rv = createIndex(row, column);

  return rv;
  }


QModelIndex StringListModel::parent(const QModelIndex &index) const {
  return QModelIndex();
  }


bool StringListModel::removeRow(int row, const QModelIndex& parent) {
  int os = sList.size();

  beginRemoveRows(QModelIndex(), row, row);
  sList.removeAt(row);
  endRemoveRows();

  return sList.size() < os;
  }


QStringList StringListModel::stringList() const {
  return sList;
  }


int StringListModel::rowCount(const QModelIndex& parent) const {
  return sList.count();
  }
