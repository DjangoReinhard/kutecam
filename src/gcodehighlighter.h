/* 
 * **************************************************************************
 * 
 *  file:       gcodehighlighter.h
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
#ifndef GCODEHIGHLIGHTER_H
#define GCODEHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>


class GCodeHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT
public:
  GCodeHighlighter(QTextDocument* parent);
  GCodeHighlighter(QObject* parent = nullptr);
  GCodeHighlighter(const GCodeHighlighter& other);

  void highlightBlock(const QString &text) override;

protected:
  void setup();

private:
  struct HighlightingRule
  {
    QRegularExpression pattern;
    QTextCharFormat format;
    };
  QVector<HighlightingRule> highlightingRules;
  };
#endif // GCODEHIGHLIGHTER_H
