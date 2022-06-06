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
 , highlightingRules(other.highlightingRules) {
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
  QTextCharFormat gcFormat;

  gcFormat.setForeground(Qt::lightGray);
  gcFormat.setFontItalic(true);
  gcFormat.setFontWeight(QFont::Normal);
  rule.pattern = QRegularExpression(QStringLiteral("\\bN\\d+\\b"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkRed);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("\\bG\\d\\d+\\.?\\d*\\b"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkYellow);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("="));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkYellow);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("(BEGIN|END)\\s+PGM\\s+\\S+\\s+MM"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkRed);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*CC\\s+"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkRed);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*CYCLE\\s+DEF\\s+\\d+"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::red);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("(?=G\\d\\D)G0"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(QColor(180, 0, 0));
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("(?=G\\d\\D)(G1|G2|G3)"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::red);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*(L|C)\\s+"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkMagenta);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("\\bM\\d+\\.?\\d*\\b"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkMagenta);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("\\bMCALL\\b|CYCLE\\d+"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkBlue);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Normal);
  rule.pattern = QRegularExpression(QStringLiteral("\\b[ABCUVXY]([+-]?\\d+[,.]?\\d*|\\[.+?\\])\\b"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::black);
  gcFormat.setFontItalic(true);
  gcFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegularExpression(QStringLiteral("\\b[WZ]([+-]?\\d+[,.]?\\d*|\\[.+?\\])\\b"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkCyan);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Normal);
  rule.pattern = QRegularExpression(QStringLiteral("\\b[EIJKLQR]([+-]?\\d+[,.]?\\d*|\\[.+?\\])\\b"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkCyan);
  gcFormat.setFontItalic(false);
  gcFormat.setFontWeight(QFont::Normal);
  rule.pattern = QRegularExpression(QStringLiteral("\\bDR[+-]|R0|RR|RL\\b"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkYellow);
  gcFormat.setFontWeight(QFont::Bold);
  gcFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("\\b[THD]\\d*[,.]?\\d*\\b"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::magenta);
  gcFormat.setFontWeight(QFont::Bold);
  gcFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("\\bF\\d*[,.]?\\d*\\b"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::magenta);
  gcFormat.setFontWeight(QFont::Bold);
  gcFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("\\bS\\d*[,.]?\\d*\\b"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::magenta);
  gcFormat.setFontWeight(QFont::Normal);
  gcFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("\\bFMAX\\b"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkCyan);
  gcFormat.setFontWeight(QFont::Normal);
  gcFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*TOOL\\s+DEF\\s+"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::cyan);
  gcFormat.setFontWeight(QFont::Bold);
  gcFormat.setFontItalic(false);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*TOOL\\s+CALL\\s+\\d+\\s+[XYZ]"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkCyan);
  gcFormat.setFontWeight(QFont::Bold);
  gcFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("^\\s*BLK\\s+FORM\\s+(0\\.1|0\\.2)\\s+(Z\\s+)?"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::cyan);
  gcFormat.setFontWeight(QFont::Bold);
  gcFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("#\\<.+?\\>"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkGreen);
  gcFormat.setFontWeight(QFont::Normal);
  gcFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral(";.*$"));
  rule.format = gcFormat;
  highlightingRules.append(rule);

  gcFormat.setForeground(Qt::darkGreen);
  gcFormat.setFontWeight(QFont::Normal);
  gcFormat.setFontItalic(true);
  rule.pattern = QRegularExpression(QStringLiteral("\\(.+?\\)"));
  rule.format = gcFormat;
  highlightingRules.append(rule);
  }
