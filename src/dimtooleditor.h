/* 
 * **************************************************************************
 * 
 *  file:       dimtooleditor.h
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
#ifndef DIMTOOLEDITOR_H
#define DIMTOOLEDITOR_H
#include <QWidget>
QT_BEGIN_NAMESPACE
namespace Ui {
class DimensionToolEditor;
}
QT_END_NAMESPACE
class ToolEntry;
class QVariant;


class DimToolEditor : public QWidget
{
  Q_OBJECT
public:
  explicit DimToolEditor(ToolEntry* toolEntry, QWidget *parent = nullptr);

  void saveTool(ToolEntry* toolEntry);
  void setTool(ToolEntry* toolEntry);

public slots:
  void cbColletChanged(const QVariant& ndx);
  bool eventFilter(QObject* o, QEvent* event) override;

private:
  Ui::DimensionToolEditor* ui;
  ToolEntry*               toolEntry;
  };
#endif // DIMTOOLEDITOR_H
