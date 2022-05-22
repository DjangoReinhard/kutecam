/* 
 * **************************************************************************
 * 
 *  file:       pluginlistmodel.h
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
#ifndef PLUGINLISTMODEL_H
#define PLUGINLISTMODEL_H
#include <QAbstractListModel>


class PluginListModel : public QAbstractListModel
{
public:
  explicit PluginListModel(QObject* parent = nullptr);

  void               clear();
  QVariant           data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant           data(int row, int column, int role = Qt::DisplayRole) const;
  int                rowCount(const QModelIndex &parent = QModelIndex()) const override;
  void               setData(const QString& key, const QString& path);
  QString            value(const QString& key);

private:
  QMap<QString, QString> plugins;
  };

#endif // PLUGINLISTMODEL_H
