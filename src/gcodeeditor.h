/* 
 * **************************************************************************
 * 
 *  file:       gcodeeditor.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    19.1.2022 by Django Reinhard
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
#ifndef GCODEEDITOR_H
#define GCODEEDITOR_H

#include <QPlainTextEdit>

class LineNumberArea;


class GCodeEditor : public QPlainTextEdit
{
  Q_OBJECT
public:
  explicit GCodeEditor(QWidget* parent = nullptr);
  virtual ~GCodeEditor();

  void paintLineNumbers(QPaintEvent* event);
  int  lineNumberAreaWidth();
  bool loadFile(QVariant fileName);
  void highlightCurrentLine();
  int  numLines() const { return document()->blockCount(); }

protected:
  void resizeEvent(QResizeEvent* event) override;

private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void updateLineNumberArea(const QRect& rect, int dy);

private:
  QWidget* lineNumberArea;
  };


class LineNumberArea : public QWidget
{
public:
  LineNumberArea(GCodeEditor *editor)
   : QWidget(editor)
   , editor(editor) {
    setFont(editor->font());
    }
  QSize sizeHint() const override {
    return QSize(editor->lineNumberAreaWidth(), 0);
    }

protected:
  void paintEvent(QPaintEvent *event) override {
    editor->paintLineNumbers(event);
    }

private:
  GCodeEditor* editor;
  };
#endif // GCODEEDITOR_H
