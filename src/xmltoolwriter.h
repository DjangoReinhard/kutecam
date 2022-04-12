/* 
 * **************************************************************************
 * 
 *  file:       xmltoolwriter.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    27.2.2022 by Django Reinhard
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
#ifndef XMLTOOLWRITER_H
#define XMLTOOLWRITER_H
#include <QObject>
class QString;
class QTextStream;
class ToolEntry;


class XmlToolWriter : public QObject
{
  Q_OBJECT
public:
  explicit XmlToolWriter(QObject *parent = nullptr);

  void write(const QString& fileName, const QVector<ToolEntry*>& tools);

protected:
  void writeLibrary(QTextStream& out, const QVector<ToolEntry*>& tools);
  void writeTool(QTextStream& out, ToolEntry* t);
  void writeProperty(QTextStream& out, const QString& name, const QString v, bool doubleIndent = false);
  void writeProperty(QTextStream& out, const QString& name, double v, bool doubleIndent = false);
  };
#endif // XMLTOOLWRITER_H
