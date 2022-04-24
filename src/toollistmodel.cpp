/* 
 * **************************************************************************
 * 
 *  file:       toollistmodel.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    7.4.2022 by Django Reinhard
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
#include "toollistmodel.h"
#include "toolentry.h"
#include <QDebug>


ToolListModel::ToolListModel(QObject *parent)
 : QAbstractListModel(parent) {
  if (!noTool) noTool = new ToolEntry(0, tr("-- please select --"));
  }


void ToolListModel::add(ToolEntry* tool) {
  int row = tList.count();

  beginInsertRows(QModelIndex(), row, row);
  tList.append(tool);
  endInsertRows();
  }


QVariant ToolListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();
  else if (role == Qt::DisplayRole) {
    ToolEntry* te = tList.at(index.row());

    if (!te) return QVariant();
    return te->toString();
    }
  return QVariant();
  }


int ToolListModel::findToolNum(int toolNum) {
  for (int i=0; i < tList.size(); ++i) {
      if (tList.at(i)->toolNumber() == toolNum) return i;
      }
  return -1;
  }


QVariant ToolListModel::headerData(int section, Qt::Orientation orientation, int role) const {
  return tr("Tools");
  }


int ToolListModel::rowCount(const QModelIndex& parent) const {
  return tList.count();
  }


void ToolListModel::setData(const QVector<ToolEntry *> &tools) {
  tList.clear();
  add(noTool);
  for (ToolEntry* t : tools) {
      add(t);
      }
  }


void ToolListModel::setupTestData() {
  ToolEntry*  te = new ToolEntry(1, "NC Center Drill");

  te->setCollet(1);
  te->setCuttingDepth(10);
  te->setFluteDiameter(16);
  te->setFluteLength(8);
  te->setFreeLength(150);
  te->setNumFlutes(2);
  te->setShankDiameter(16);
  add(te);

  te = new ToolEntry(2, "50mm Shoulder Endmill");
  te->setCollet(1);
  te->setCuttingDepth(100);
  te->setFluteDiameter(50);
  te->setFluteLength(7);
  te->setFreeLength(100);
  te->setNumFlutes(4);
  te->setShankDiameter(45);
  add(te);

  te = new ToolEntry(3, "80mm Shoulder Endmill");
  te->setCollet(1);
  te->setCuttingDepth(150);
  te->setFluteDiameter(80);
  te->setFluteLength(7);
  te->setFreeLength(200);
  te->setNumFlutes(7);
  te->setShankDiameter(72);
  add(te);

  te = new ToolEntry(4, "12mm HPC Endmill");
  te->setCollet(1);
  te->setCuttingDepth(35);
  te->setFluteDiameter(12);
  te->setFluteLength(26);
  te->setFreeLength(50);
  te->setNumFlutes(4);
  te->setShankDiameter(12.5);
  add(te);

  te = new ToolEntry(5, "6,8mm Drill");
  te->setCollet(1);
  te->setCuttingDepth(125);
  te->setFluteDiameter(6.6);
  te->setFluteLength(95);
  te->setFreeLength(125);
  te->setNumFlutes(2);
  te->setShankDiameter(6.8);
  add(te);

  te = new ToolEntry(6, "M8mm Tap");
  te->setCollet(1);
  te->setCuttingDepth(85);
  te->setFluteDiameter(8);
  te->setFluteLength(37);
  te->setFreeLength(85);
  te->setNumFlutes(3);
  te->setShankDiameter(6);
  add(te);

  te = new ToolEntry(7, "12,5mm Drill");
  te->setCollet(1);
  te->setCuttingDepth(150);
  te->setFluteDiameter(12.5);
  te->setFluteLength(120);
  te->setFreeLength(150);
  te->setNumFlutes(2);
  te->setShankDiameter(12.4);
  add(te);

  te = new ToolEntry(8, "30mm Insert Drill");
  te->setCollet(1);
  te->setCuttingDepth(190);
  te->setFluteDiameter(30);
  te->setFluteLength(6);
  te->setFreeLength(190);
  te->setNumFlutes(2);
  te->setShankDiameter(27.5);
  add(te);
  qDebug() << "tool model contains" << tList.count() << "tools";
  }


ToolEntry* ToolListModel::tool(int row) {
  if (row < 0 || row >= tList.count()) return nullptr;
  return tList.at(row);
  }


QVector<ToolEntry*> ToolListModel::tools() const {
  return tList;
  }


ToolEntry* ToolListModel::noTool = nullptr;
