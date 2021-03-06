/* 
 * **************************************************************************
 * 
 *  file:       gcodewriter.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    11.4.2022 by Django Reinhard
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
#ifndef GCODEWRITER_H
#define GCODEWRITER_H
#include <QVector>
class Bnd_Box;
class QString;
class Operation;
class PostProcessor;
class ToolEntry;
class QTextStream;


class GCodeWriter
{
public:
  explicit GCodeWriter(PostProcessor* pp);

  int  processAllInOne(const QString& fileName, const Bnd_Box& wpBounds, const QVector<Operation*>& operations);
  int  processSingleOPs(const QString& fileName, const Bnd_Box& wpBounds, const QVector<Operation*>& operations, bool genTC = false);

protected:
  void processOperation(QTextStream& out, int n, const QString& opName, const Bnd_Box& wpBounds, const Operation* op, const Operation* nxtOP, bool genTC = false);
  void processDrillTargets(QTextStream& out, const Operation* op, int first, ToolEntry* curTool);
  void processPathTargets(QTextStream& out, const Operation* op, int first, ToolEntry* curTool);
  void writeLine(QTextStream& out, const QString& line = QString());

private:
  PostProcessor*  pp;
  double          rotA;
  double          rotB;
  double          rotC;
  };
#endif // GCODEWRITER_H
