/* 
 * **************************************************************************
 * 
 *  file:       toolpage.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    4.4.2022 by Django Reinhard
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
#ifndef TOOLPAGE_H
#define TOOLPAGE_H
#include "applicationwindow.h"
class QListView;
class QItemSelection;
class StringListModel;
class ToolEditor;
class ToolEntry;


class ToolPage : public ApplicationWindow
{
  Q_OBJECT
public:
  explicit ToolPage(StringListModel* matModel, QWidget *parent = nullptr);

  void addTool(ToolEntry* te);
  bool eventFilter(QObject* o, QEvent* event);
  void toolSelected(const QItemSelection& selected, const QItemSelection& deselected);
  void saveTools();
  void loadTools();
  bool readTools(const QString& fileName);

private:
  QListView*       tools;
  ToolEditor*      edit;
  StringListModel* matModel;
  };
#endif // TOOLPAGE_H
