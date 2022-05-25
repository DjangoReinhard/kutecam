/* 
 * **************************************************************************
 * 
 *  file:       editorpage.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    22.5.2022 by Django Reinhard
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
#ifndef EDITORPAGE_H
#define EDITORPAGE_H
#include <QWidget>
QT_BEGIN_NAMESPACE
namespace Ui {
class GCodeEditorPage;
}
class GCodeEditor;
class GCodeHighlighter;


class EditorPage : public QWidget
{
  Q_OBJECT
public:
  explicit EditorPage(QWidget *parent = nullptr);

  void loadFile(const QString& fileName);

protected:
  QString chooseGCodeFile(QWidget* parent = nullptr);
  void    showEvent(QShowEvent *event);

protected slots:
  void openFile();
  void saveFile();
  void dirtyChanged(bool dirty);

private:
  Ui::GCodeEditorPage* ui;
  GCodeEditor*         ed;
  GCodeHighlighter*    gh;
  QString              fileName;
  };
#endif // EDITORPAGE_H
