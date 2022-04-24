/* 
 * **************************************************************************
 * 
 *  file:       targetdeflistmodel.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    23.4.2022 by Django Reinhard
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
#include "targetdeflistmodel.h"
#include "targetdefinition.h"


TargetDefListModel::TargetDefListModel(std::vector<TargetDefinition*>* list, QObject* parent)
 : QAbstractListModel(parent)
 , pList(list) {
  }


void TargetDefListModel::append(TargetDefinition* td) {
  int newRow = pList->size();

  beginInsertRows(QModelIndex(), newRow, newRow);
  pList->push_back(td);
  endInsertRows();
  }


void TargetDefListModel::clear() {
  beginResetModel();
  pList->clear();
  endResetModel();
  }


QVariant TargetDefListModel::data(int row, int column, int role) const {
  QModelIndex mi = createIndex(row, column);

  return data(mi, role);
  }


QVariant TargetDefListModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole) return QVariant();
  if (index.row() >= pList->size()) return QVariant();
  TargetDefinition* td = pList->at(index.row());

  if (!index.column()) return td->toString();
  return QVariant();
  }


TargetDefinition* TargetDefListModel::item(int row) const {
  return pList->at(row);
  }


std::vector<TargetDefinition*>& TargetDefListModel::itemList() {
  return *pList;
  }


bool TargetDefListModel::removeRows(int row, int count, const QModelIndex& parent) {
  int os = pList->size();

  beginRemoveRows(parent, row, row);
  pList->erase(pList->begin() + row);
  endRemoveRows();

  return pList->size() < os;
  }


void TargetDefListModel::replaceData(std::vector<TargetDefinition *>* data) {
  if (!data) return;
  beginResetModel();
  pList = data;
  endResetModel();
  }


int TargetDefListModel::rowCount(const QModelIndex &parent) const {
  return pList->size();
  }


void TargetDefListModel::sort(int column, Qt::SortOrder order) {
  if (order == Qt::AscendingOrder)
     std::sort(pList->begin(), pList->end(), TargetDefinition::compareASC);
  else
    std::sort(pList->begin(), pList->end(), TargetDefinition::compareDESC);
  }
