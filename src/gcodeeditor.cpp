/*
 * **************************************************************************
 *
 *  file:       gcodeeditor.cpp
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
#include "gcodeeditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QFile>
#include <QDebug>


GCodeEditor::GCodeEditor(QWidget* parent)
 : QPlainTextEdit(parent) {
  lineNumberArea = new LineNumberArea(this);

  connect(this, &GCodeEditor::blockCountChanged,     this, &GCodeEditor::updateLineNumberAreaWidth);
  connect(this, &GCodeEditor::updateRequest,         this, &GCodeEditor::updateLineNumberArea);
  connect(this, &GCodeEditor::cursorPositionChanged, this, &GCodeEditor::highlightCurrentLine);

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
  setCenterOnScroll(true);
  }


GCodeEditor::~GCodeEditor() {
  }


int GCodeEditor::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, document()->blockCount());

  while (max >= 10) {
        max /= 10;
        ++digits;
        }
  int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

  return space;
  }


void GCodeEditor::updateLineNumberAreaWidth(int /* bc */) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
  }


void GCodeEditor::updateLineNumberArea(const QRect& rect, int dy) {
  if (dy) lineNumberArea->scroll(0, dy);
  else    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect()))
     updateLineNumberAreaWidth(0);
  }


void GCodeEditor::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);
  QRect cr = contentsRect();

  lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
  }


bool GCodeEditor::loadFile(QVariant name) {
  QFile file(name.toString());

  if (file.open(QFile::ReadOnly | QFile::Text)) {
     setPlainText(file.readAll());
     file.close();

     return true;
     }
  else
     qDebug() << tr("could not read file %1").arg(name.toString());
  return false;
  }


void GCodeEditor::highlightCurrentLine() {
  QList<QTextEdit::ExtraSelection> extraSelections;
  QTextEdit::ExtraSelection selection;
  QColor bgHLLine = QColor::fromRgb(0, 0, 255);   //ValueManager().getValue("cfgBg" + Config().nameOf(Config::GuiElem::LineHL)).value<QColor>();
  QColor fgHLLine = QColor::fromRgb(255, 255, 0); //ValueManager().getValue("cfgFg" + Config().nameOf(Config::GuiElem::LineHL)).value<QColor>();

  selection.format.setBackground(bgHLLine);
  selection.format.setForeground(fgHLLine);
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  selection.cursor = textCursor();
  selection.cursor.clearSelection();
  extraSelections.append(selection);
  setExtraSelections(extraSelections);
  }


void GCodeEditor::paintLineNumbers(QPaintEvent *e) {
  QPainter painter(lineNumberArea);
//  QColor   bgLineNum = ValueManager().getValue("cfgBg" + Config().nameOf(Config::GuiElem::EdLinNum)).value<QColor>();
//  QColor   fgLineNum = ValueManager().getValue("cfgFg" + Config().nameOf(Config::GuiElem::EdLinNum)).value<QColor>();
  QColor bgLineNum = QColor::fromRgb(170, 170, 170);
  QColor fgLineNum = QColor::fromRgb(255, 255, 255);

  painter.fillRect(e->rect(), bgLineNum);
  painter.setFont(font());
  QTextBlock block    = firstVisibleBlock();        // lines == block
  int        blockNum = block.blockNumber();
  int        top      = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int        bottom   = top + qRound(blockBoundingRect(block).height());

  while (block.isValid() && top <= e->rect().bottom()) {
        if (block.isVisible() && bottom >= e->rect().top()) {
           QString lineNum = QString::number(blockNum + 1);

           painter.setPen(fgLineNum);
           painter.drawText(0
                          , top
                          , lineNumberArea->width()
                          , fontMetrics().height()
                          , Qt::AlignRight
                          , lineNum);
           }
        block  = block.next();
        top    = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNum;
        }
  }
