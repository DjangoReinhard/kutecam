/* 
 * **************************************************************************
 * 
 *  file:       tooleditor.h
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
#ifndef TOOLEDITOR_H
#define TOOLEDITOR_H
#include "toolentry.h"
#include <QWidget>
QT_BEGIN_NAMESPACE
namespace Ui {
class ToolEditor;
}
QT_END_NAMESPACE
class CutParmToolEditor;
class DimToolEditor;
class ToolEntry;
class CutParamListModel;
class CuttingParameters;
class StringListModel;
class ToolListModel;
class QItemSelection;
class QLineEdit;
class QVariant;


class ToolEditor : public QWidget
{
  Q_OBJECT
public:
  explicit ToolEditor(StringListModel* matModel, ToolListModel* tools, QWidget *parent = nullptr);

  void initialize();
  void saveTool(ToolEntry* tool);
  void setTool(ToolEntry* tool);

public slots:
  void toolSelected(const QItemSelection &selected, const QItemSelection &deselected);

protected:
  double readDouble(QLineEdit* edit);

private:
  Ui::ToolEditor*    ui;
  ToolEntry          toolEntry;
  CutParamListModel* model;
  StringListModel*   matModel;
  ToolEntry*         curTool;
  CuttingParameters* curCutParams;
  ToolListModel*     tools;
  DimToolEditor*     edDim;
  CutParmToolEditor* edCP;
  };
#endif // TOOLEDITOR_H
