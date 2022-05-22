/* 
 * **************************************************************************
 * 
 *  file:       pluginlistmodel.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    17.5.2022 by Django Reinhard
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
#include "pluginlistmodel.h"


PluginListModel::PluginListModel(QObject* parent)
 : QAbstractListModel(parent) {
  }


void PluginListModel::clear() {
  plugins.clear();
  }


QVariant PluginListModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole) return QVariant();
  if (index.row() >= plugins.size()) return QVariant();
  QString key = plugins.keys().at(index.row());

  if (!index.column()) return key;
  return plugins.value(key);
  }


QVariant PluginListModel::data(int row, int column, int role) const {
  QModelIndex mi = createIndex(row, column);

  return data(mi, role);
  }


int PluginListModel::rowCount(const QModelIndex& parent) const {
  return plugins.size();
  }


void PluginListModel::setData(const QString& key, const QString& path) {
  beginResetModel();
  plugins[key] = path;
  endResetModel();
  }


QString PluginListModel::value(const QString& key) {
  return plugins[key];
  }
