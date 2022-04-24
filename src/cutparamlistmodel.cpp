/* 
 * **************************************************************************
 * 
 *  file:       cutparamlistmodel.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    22.3.2022 by Django Reinhard
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
#include "cutparamlistmodel.h"
#include "cuttingparameters.h"


CutParamListModel::CutParamListModel(QObject *parent)
 : QAbstractListModel(parent)
  {

  }


void CutParamListModel::add(CuttingParameters* cp) {
  int row = cpList.size();

  beginInsertRows(QModelIndex(), row, row);
  cpList.append(cp);
  endInsertRows();
  }


CuttingParameters* CutParamListModel::cutParameter(int row) const {
  return cpList.at(row);
  }


QVariant CutParamListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();
  else if (role == Qt::DisplayRole) {
     CuttingParameters* cp = cpList.at(index.row());

     if (!cp) return QVariant();
     return cp->name();
     }
  return QVariant();
  }


QVariant CutParamListModel::headerData(int section, Qt::Orientation orientation, int role) const {
  return tr("Cut Parameter");
  }


int CutParamListModel::rowCount(const QModelIndex &parent) const {
  return cpList.size();
  }


void CutParamListModel::replace(QVector<CuttingParameters *> list) {
  beginResetModel();
  cpList = list;
  endResetModel();
  }


QVector<CuttingParameters*> CutParamListModel::list() {
  return cpList;
  }
