/* 
 * **************************************************************************
 * 
 *  file:       xmltoolwriter.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    22.4.2022 by Django Reinhard
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
#include "xmltoolwriter.h"
#include "toolentry.h"
#include "cuttingparameters.h"
#include <QFile>
#include <QTextStream>


XmlToolWriter::XmlToolWriter(QObject* parent)
 : QObject(parent) {
  }


void XmlToolWriter::write(const QString& fileName, const QVector<ToolEntry *>& tools) {
  QFile toolFile(fileName);

  if (toolFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
     QTextStream out(&toolFile);

     writeLibrary(out, tools);
     toolFile.flush();
     toolFile.close();
     }
  }


void XmlToolWriter::writeLibrary(QTextStream& out, const QVector<ToolEntry*>& tools) {
  out << "<?xml version=\"1.0\" ?>\n<ToolLibrary>\n";
  for (ToolEntry* t : tools) {
      if (t->toolNumber() < 1) continue;
      writeTool(out, t);
      }
  out << "</ToolLibrary>\n";
  }


void XmlToolWriter::writeProperty(QTextStream& out, const QString &name, const QString v, bool doubleIndent) {
  if (doubleIndent) out << "    ";
  out << QString("    <%1>%2</%1>\n").arg(name).arg(v);
  }


void XmlToolWriter::writeProperty(QTextStream& out, const QString &name, double v, bool doubleIndent) {
  if (doubleIndent) out << "    ";
  out << QString("    <%1>%2</%1>\n").arg(name).arg(v, 0, 'f', 2);
  }


void XmlToolWriter::writeTool(QTextStream& out, ToolEntry* t) {
  if (!t) return;
  out << QString("  <ToolDefinition Number=\"%1\">\n").arg(t->toolNumber());

  writeProperty(out, "Name",          t->toolName());
  writeProperty(out, "Collet",        t->collet());
  writeProperty(out, "FluteDiameter", t->fluteDiameter());
  writeProperty(out, "FluteLength",   t->fluteLength());
  writeProperty(out, "Flutes",        t->numFlutes());
  writeProperty(out, "CuttingDepth",  t->cuttingDepth());
  writeProperty(out, "ShankDiameter", t->shankDiameter());
  writeProperty(out, "FreeLength",    t->freeLength());
  QVector<CuttingParameters*> cpList = t->cutParameters();
  CuttingParameters*          cp;
  int                         mx = cpList.size();

  for (int i=0; i < mx; ++i) {
      cp = cpList.at(i);

      out << QString("    <CuttingParameters num=\"%1\">\n").arg(i);
      writeProperty(out, "Material", cp->name(), true);
      writeProperty(out, "Speed", cp->cuttingSpeed(), true);
      writeProperty(out, "Feed",  cp->toothFeed(), true);
      writeProperty(out, "Width", cp->widthOfCut(), true);
      writeProperty(out, "Depth", cp->depthOfCut(), true);
      out << "    </CuttingParameters>\n";
      }
  out << "  </ToolDefinition>\n";
  }
