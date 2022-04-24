/* 
 * **************************************************************************
 * 
 *  file:       targetdeflistmodel.h
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
#ifndef TARGETDEFLISTMODEL_H
#define TARGETDEFLISTMODEL_H
#include <QAbstractListModel>
class TargetDefinition;
class QModelIndex;


class TargetDefListModel : public QAbstractListModel
{
public:
  TargetDefListModel(std::vector<TargetDefinition*>* list, QObject* parent = nullptr);

  virtual void                    append(TargetDefinition* td);
  virtual void                    clear();
  virtual QVariant                data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  virtual QVariant                data(int row, int column, int role = Qt::DisplayRole) const;
  virtual TargetDefinition*       item(int row = 0) const;
  std::vector<TargetDefinition*>& itemList();
  virtual bool                    removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
  virtual void                    replaceData(std::vector<TargetDefinition*>* data);
  virtual int                     rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual void                    sort(int column = 0, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
  std::vector<TargetDefinition*>* pList;
  };
#endif // TARGETDEFLISTMODEL_H
