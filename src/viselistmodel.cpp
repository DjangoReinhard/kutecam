/* 
 * **************************************************************************
 * 
 *  file:       viselistmodel.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    20.3.2022 by Django Reinhard
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
#include "viselistmodel.h"
#include "viseentry.h"
#include <QDebug>


ViseListModel::ViseListModel(QObject *parent)
 : QAbstractItemModel(parent) {
  }


void ViseListModel::add(ViseEntry* tool) {
  int row = vList.count();

  beginInsertRows(QModelIndex(), row, row);
  vList.append(tool);
  endInsertRows();
  }


int ViseListModel::columnCount(const QModelIndex& parent) const {
  return 1;
  }


QVariant ViseListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();
  else if (role == Qt::DisplayRole) {
    ViseEntry* ve = vList.at(index.row());

    if (!ve) return QVariant();
    switch (index.column()) {
      case 0: return ve->name();
      case 1: return ve->left();
      case 2: return ve->middle();
      case 3: return ve->right();
      }
    }
  return QVariant();
  }


ViseEntry* ViseListModel::find(const QString& name) const {
  ViseEntry* ve;

  qDebug() << "search vise of name:" << name;
  for (int i=0; i < vList.size(); ++i) {
      ve = vList.at(i);
      if (!ve->name().compare(name))
         return ve;
      }
  qDebug() << "Oups, no vise for name:" << name;

  return nullptr;
  }


QVariant ViseListModel::headerData(int section, Qt::Orientation orientation, int role) const {
  return tr("Vises");
  }


QModelIndex ViseListModel::index(int row, int column, const QModelIndex& parent) const {
  QModelIndex rv = createIndex(row, column, vList.at(row));

  return rv;
  }


void ViseListModel::insertData(ViseEntry* ve) {
  if (!ve) return;
  vList.append(ve);
  }


QModelIndex ViseListModel::parent(const QModelIndex &index) const {
  return QModelIndex(); //index.parent();
  }


bool ViseListModel::removeRow(int row, const QModelIndex& parent) {
  beginRemoveRows(QModelIndex(), row, row);
  vList.remove(row);
  endRemoveRows();
  return false;
  }


int ViseListModel::rowCount(const QModelIndex& parent) const {
  return vList.count();
  }


void ViseListModel::setData(const QVector<ViseEntry *>& vises) {
  for (ViseEntry* ve : vises) {
      add(ve);
      }
  }


ViseEntry* ViseListModel::vise(int row) {
  if (row < 0 || row >= vList.count()) return nullptr;
  return vList.at(row);
  }


QVector<ViseEntry*> ViseListModel::vises() const {
  return vList;
  }
