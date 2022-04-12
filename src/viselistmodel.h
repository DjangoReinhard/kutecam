/* 
 * **************************************************************************
 * 
 *  file:       viselistmodel.h
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
#ifndef VISELISTMODEL_H
#define VISELISTMODEL_H
#include <QAbstractListModel>
class ViseEntry;


class ViseListModel : public QAbstractItemModel
{
public:
  explicit ViseListModel(QObject *parent = nullptr);
  virtual ~ViseListModel() = default;

  virtual void                add(ViseEntry* tool);
  virtual int                 columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant            data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual ViseEntry*          find(const QString& name) const;
  virtual QVariant            headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  virtual QModelIndex         index(int row, int column, const QModelIndex& parent) const override;
  virtual void                insertData(ViseEntry* tool);
  virtual QModelIndex         parent(const QModelIndex &index) const override;
  virtual bool                removeRow(int row, const QModelIndex& parent = QModelIndex());
  virtual int                 rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual void                setData(const QVector<ViseEntry*>& tools);
  virtual ViseEntry*          vise(int row);
  virtual QVector<ViseEntry*> vises() const;

private:
  QVector<ViseEntry*> vList;
  };
#endif // VISELISTMODEL_H
