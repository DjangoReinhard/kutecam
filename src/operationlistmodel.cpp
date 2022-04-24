/* 
 * **************************************************************************
 * 
 *  file:       operationlistmodel.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    17.4.2022 by Django Reinhard
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
#include "operationlistmodel.h"
#include "operation.h"
#include <QDebug>


OperationListModel::OperationListModel(QObject *parent)
 : QAbstractListModel(parent) {
  }


void OperationListModel::append(Operation* op) {
  int row = list.count();

  beginInsertRows(QModelIndex(), row, row);
  list.push_back(op);
  endInsertRows();
  }


void OperationListModel::clear() {
  beginResetModel();
  list.clear();
  endResetModel();
  }


QVariant OperationListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();
  else if (role == Qt::DisplayRole) {
    Operation* op = list.at(index.row());

    if (!op) return QVariant();
    return op->toString();
    }
  return QVariant();
  }


QVariant OperationListModel::headerData(int section, Qt::Orientation orientation, int role) const {
  return tr("Operations");
  }


void OperationListModel::insertData(Operation* op) {
  if (!op) return;
  list.append(op);
  }


void OperationListModel::moveDown(const QModelIndex &index) {
  if (index.row() > (list.count() - 2)) return;
  beginResetModel();
  list.move(index.row(), index.row() + 1);
  endResetModel();
  }


void OperationListModel::moveUp(const QModelIndex &index) {
  if (index.row() < 1) return;
  beginResetModel();
  list.move(index.row(), index.row() - 1);
  endResetModel();
  }


Operation* OperationListModel::operation(int row) {
  if (row < 0 || row >= list.count()) return nullptr;
  return list.at(row);
  }


QVector<Operation*> OperationListModel::operations() const {
  return list;
  }


bool OperationListModel::removeRows(int row, int count, const QModelIndex& parent) {
  int os = list.count();

  beginRemoveRows(parent, row, row);
  list.remove(row);
  endRemoveRows();

  return list.count() < os;
  }


int OperationListModel::rowCount(const QModelIndex& parent) const {
  return list.count();
  }


void OperationListModel::setData(const QVector<Operation *> &ops) {
  clear();
  beginResetModel();
  for (Operation* o : ops) {
      append(o);
      }
  endResetModel();
  }


void OperationListModel::setData(const std::vector<Operation*>& ops) {
  clear();
  beginResetModel();
  for (Operation* o : ops) {
      append(o);
      }
  endResetModel();
  }
