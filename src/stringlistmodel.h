/*
 * **************************************************************************
 *
 *  file:       stringlistmodel.h
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
#ifndef STRINGLISTMODEL_H
#define STRINGLISTMODEL_H
#include <QAbstractItemModel>
#include <QStringList>
#include <QVector>


class StringListModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit StringListModel(const QStringList& sl, QObject *parent = nullptr);

  virtual void                add(const QString& material);
  virtual int                 columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant            data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual QVariant            headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  virtual QModelIndex         index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  virtual QModelIndex         parent(const QModelIndex &index) const override;
  virtual bool                removeRow(int row, const QModelIndex& parent = QModelIndex());
  virtual int                 rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual QStringList         stringList() const;

private:
  QStringList sList;
  };
#endif // STRINGLISTMODEL_H
