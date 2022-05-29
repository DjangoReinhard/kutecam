/* 
 * **************************************************************************
 * 
 *  file:       cutparmtooleditor.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    15.5.2022 by Django Reinhard
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
#ifndef CUTPARMTOOLEDITOR_H
#define CUTPARMTOOLEDITOR_H
#include <QWidget>
QT_BEGIN_NAMESPACE
namespace Ui {
class CuttingParameters;
}
QT_END_NAMESPACE
class CutParamListModel;
class CuttingParameters;
class StringListModel;
class ToolEntry;
class QItemSelection;


class CutParmToolEditor : public QWidget
{
  Q_OBJECT
public:
  explicit CutParmToolEditor(StringListModel* matModel, ToolEntry* toolEntry, QWidget *parent = nullptr);

  void setTool(ToolEntry* toolEntry);
  void saveTool(ToolEntry* toolEntry);

public slots:
  void addMaterial();
  void delMaterial();
  void cutParamChanged(const QItemSelection &selected, const QItemSelection &deselected);
  void docChanged(const QVariant& v);
  void wocChanged(const QVariant& v);
  void speedChanged(const QVariant& v);
  void feedChanged(const QVariant& v);

protected:
  void calcSpeed();

private:
  Ui::CuttingParameters* ui;
  CutParamListModel*     model;
  StringListModel*       matModel;
  CuttingParameters*     curCutParams;
  ToolEntry*             toolEntry;
  };
#endif // CUTPARMTOOLEDITOR_H
