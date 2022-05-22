/* 
 * **************************************************************************
 * 
 *  file:       gcodehighlighter.cpp
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
#include "gcodehighlighter.h"


GCodeHighlighter::GCodeHighlighter(QTextDocument* parent)
 : QSyntaxHighlighter(parent) {
  setup();
  }


GCodeHighlighter::GCodeHighlighter(QObject* parent)
 : QSyntaxHighlighter(parent) {
  setup();
  }


GCodeHighlighter::GCodeHighlighter(const GCodeHighlighter& other)
 : QSyntaxHighlighter(other.parent())
 , highlightingRules(other.highlightingRules)
 , fmtNumber(other.fmtNumber)
 , fmtGCode(other.fmtGCode)
 , fmtMotion(other.fmtMotion)
 , fmtMCode(other.fmtMCode)
 , fmtPos(other.fmtPos)
 , fmtXPos(other.fmtXPos)
 , fmtTool(other.fmtTool)
 , fmtSpeed(other.fmtSpeed)
 , fmtVar(other.fmtVar)
 , fmtComment(other.fmtComment)
 , fmtLineComment(other.fmtLineComment) {
  }


void GCodeHighlighter::highlightBlock(const QString& text) {
  for (const HighlightingRule& rule : qAsConst(highlightingRules)) {
      QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);

      while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();

            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
      }
  }


void GCodeHighlighter::setup() {
  HighlightingRule rule;

  fmtNumber.setForeground(Qt::lightGray);
  fmtNumber.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("\\bN\\d+\\b"));
  rule.format = fmtNumber;
  highlightingRules.append(rule);

  fmtGCode.setForeground(Qt::darkRed);
  fmtGCode.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("\\bG\\d\\d+\\.?\\d*\\b"));
  rule.format = fmtGCode;
  highlightingRules.append(rule);

  fmtGCode.setForeground(Qt::darkYellow);
  fmtGCode.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("="));
  rule.format = fmtGCode;
  highlightingRules.append(rule);

  fmtGCode.setForeground(Qt::darkYellow);
  fmtGCode.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("(BEGIN|END)\\s+PGM\\s+\\S+\\s+MM"));
  rule.format = fmtGCode;
  highlightingRules.append(rule);

  fmtGCode.setForeground(Qt::darkRed);
  fmtGCode.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*CC\\s+"));
  rule.format = fmtGCode;
  highlightingRules.append(rule);

  fmtGCode.setForeground(Qt::darkRed);
  fmtGCode.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*CYCLE\\s+DEF\\s+\\d+"));
  rule.format = fmtGCode;
  highlightingRules.append(rule);

  fmtMotion.setForeground(Qt::red);
  fmtMotion.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("(?=G\\d\\D)G0"));
  rule.format = fmtMotion;
  highlightingRules.append(rule);

  fmtMotion.setForeground(QColor(180, 0, 0));
  fmtMotion.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("(?=G\\d\\D)(G1|G2|G3)"));
  rule.format = fmtMotion;
  highlightingRules.append(rule);

  fmtMotion.setForeground(Qt::red);
  fmtMotion.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*(L|C)\\s+"));
  rule.format = fmtMotion;
  highlightingRules.append(rule);

  fmtMCode.setForeground(Qt::darkMagenta);
  fmtMCode.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("\\bM\\d+\\.?\\d*\\b"));
  rule.format = fmtMCode;
  highlightingRules.append(rule);

  fmtMCode.setForeground(Qt::darkMagenta);
  fmtMCode.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("\\bMCALL\\b|CYCLE\\d+"));
  rule.format = fmtMCode;
  highlightingRules.append(rule);

  fmtPos.setForeground(Qt::darkBlue);
  rule.pattern = QRegularExpression(QStringLiteral("\\b[ABCUVWXYZ]([+-]?\\d+[,.]?\\d*|\\[.+?\\])\\b"));
  rule.format = fmtPos;
  highlightingRules.append(rule);

  fmtXPos.setForeground(Qt::darkCyan);
  rule.pattern = QRegularExpression(QStringLiteral("\\b[EIJKLQR]([+-]?\\d+[,.]?\\d*|\\[.+?\\])\\b"));
  rule.format = fmtXPos;
  highlightingRules.append(rule);

  fmtXPos.setForeground(Qt::darkCyan);
  rule.pattern = QRegularExpression(QStringLiteral("\\bDR[+-]|R0|RR|RL\\b"));
  rule.format = fmtXPos;
  highlightingRules.append(rule);

  fmtTool.setForeground(Qt::darkYellow);
  fmtTool.setFontWeight(QFont::Bold);
  fmtTool.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("\\b[THD]\\d*[,.]?\\d*\\b"));
  rule.format = fmtTool;
  highlightingRules.append(rule);

  fmtFeed.setForeground(Qt::magenta);
  fmtFeed.setFontWeight(QFont::Bold);
  fmtFeed.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("\\bF\\d*[,.]?\\d*\\b"));
  rule.format = fmtFeed;
  highlightingRules.append(rule);

  fmtSpeed.setForeground(Qt::magenta);
  fmtSpeed.setFontWeight(QFont::Bold);
  fmtSpeed.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("\\bS\\d*[,.]?\\d*\\b"));
  rule.format = fmtSpeed;
  highlightingRules.append(rule);

  fmtSpeed.setForeground(Qt::magenta);
  fmtSpeed.setFontWeight(QFont::Normal);
  fmtSpeed.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("\\bFMAX\\b"));
  rule.format = fmtSpeed;
  highlightingRules.append(rule);

  fmtVar.setForeground(Qt::darkCyan);
  fmtVar.setFontWeight(QFont::Normal);
  fmtVar.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*TOOL\\s+DEF\\s+"));
  rule.format = fmtVar;
  highlightingRules.append(rule);

  fmtVar.setForeground(Qt::cyan);
  fmtVar.setFontWeight(QFont::Bold);
//  fmtVar.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*TOOL\\s+CALL\\s+\\d+\\s+[XYZ]"));
  rule.format = fmtVar;
  highlightingRules.append(rule);

  fmtVar.setForeground(Qt::darkCyan);
  fmtVar.setFontWeight(QFont::Bold);
  fmtVar.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*BLK\\s+FORM\\s+(0\\.1|0\\.2)\\s+(Z\\s+)?"));
  rule.format = fmtVar;
  highlightingRules.append(rule);

  fmtVar.setForeground(Qt::cyan);
  fmtVar.setFontWeight(QFont::Bold);
  fmtVar.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("#\\<.+?\\>"));
  rule.format = fmtVar;
  highlightingRules.append(rule);

  fmtComment.setForeground(Qt::darkGreen);
  fmtComment.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral(";.*$"));
  rule.format = fmtComment;
  highlightingRules.append(rule);

  fmtLineComment.setForeground(Qt::darkGreen);
  fmtLineComment.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("\\(.+?\\)"));
  rule.format = fmtLineComment;
  highlightingRules.append(rule);
  }
