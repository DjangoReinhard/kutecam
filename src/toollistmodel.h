/* 
 * **************************************************************************
 * 
 *  file:       toollistmodel.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    22.4.2022 by Django Reinhard
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
#ifndef TOOLLISTMODEL_H
#define TOOLLISTMODEL_H
#include <QAbstractListModel>
#include <QVector>
class ToolEntry;


class ToolListModel : public QAbstractListModel
{
public:
  explicit ToolListModel(QObject *parent = nullptr);
  virtual ~ToolListModel() = default;

  virtual void                add(ToolEntry* tool);
  virtual QVariant            data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  virtual int                 findToolNum(int toolNum);
  virtual QVariant            headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  virtual int                 rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual void                setupTestData();
  virtual void                setData(const QVector<ToolEntry*>& tools);
  virtual ToolEntry*          tool(int row);
  virtual QVector<ToolEntry*> tools() const;

private:
  QVector<ToolEntry*> tList;
  static ToolEntry*   noTool;
  };
#endif // TOOLLISTMODEL_H
