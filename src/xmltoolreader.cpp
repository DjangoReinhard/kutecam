/* 
 * **************************************************************************
 * 
 *  file:       xmltoolreader.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    23.3.2022 by Django Reinhard
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
#include "xmltoolreader.h"
#include "toolentry.h"
#include "cuttingparameters.h"
#include <QDomDocument>
#include <QIODevice>
#include <QDebug>


XmlToolReader::XmlToolReader(QObject *parent)
 : QObject(parent) {
  }


void XmlToolReader::processDoc(const QDomDocument& doc, QVector<ToolEntry*>& tools) {
  ToolEntry*          tool;
  QDomNodeList        xTools = doc.elementsByTagName("ToolDefinition");

  for (int i=0; i < xTools.count(); ++i) {
      QDomNode xt = xTools.item(i);

      if (xt.isElement()) {
         tool = new ToolEntry();
         readElement(tool, xt.toElement());
         tools.append(tool);
         }
      }
  }


void XmlToolReader::processText(CuttingParameters* cp, const QDomNode& n) {
  if (!cp) return;
  const QString& rawData = n.toText().data();
  bool           ok;
  double         value = rawData.toDouble(&ok);
  const QString& name  = n.parentNode().nodeName();

  if (ok) qDebug() << "\tcolumn value:" << value;
  else    qDebug() << "\tcolumn value:" << rawData;

  if (name == "Material")   cp->setName(rawData);
  else if (name == "Speed") cp->setCuttingSpeed(value);
  else if (name == "Feed")  cp->setToothFeed(value);
  else if (name == "Width") cp->setWidthOfCut(value);
  else if (name == "Depth") cp->setDepthOfCut(value);
  }


void XmlToolReader::processText(ToolEntry* t, const QDomNode& n) {
  if (!t) return;
  const QString& rawData = n.toText().data();
  bool           ok;
  double         value = rawData.toDouble(&ok);
  const QString& name  = n.parentNode().nodeName();

  if (ok) qDebug() << "\tcolumn value:" << value;
  else    qDebug() << "\tcolumn value:" << rawData;

  if (name      == "Name")          t->setToolName(rawData);
  else if (name == "Collet")        t->setCollet(value);
  else if (name == "FluteDiameter") t->setFluteDiameter(value);
  else if (name == "FluteLength")   t->setFluteLength(value);
  else if (name == "Flutes")        t->setNumFlutes(value);
  else if (name == "CuttingDepth")  t->setCuttingDepth(value);
  else if (name == "ShankDiameter") t->setShankDiameter(value);
  else if (name == "FreeLength")    t->setFreeLength(value);
  }


QVector<ToolEntry*> XmlToolReader::read(QIODevice* io) {
  QDomDocument doc;

  doc.setContent(io);
  io->close();
  QVector<ToolEntry*> tools;

  processDoc(doc, tools);

  return tools;
  }


QVector<ToolEntry*> XmlToolReader::read(const QByteArray& ba) {
  QDomDocument doc;

  doc.setContent(ba);
  QVector<ToolEntry*> tools;

  processDoc(doc, tools);

  return tools;
  }


void XmlToolReader::readAttribute(ToolEntry* t, CuttingParameters* cp, const QDomAttr& a) {
  qDebug() << "Attribute: " << a.name() << "has value:" << a.value();

  if (a.name() == "num") {
     t->insertCuttingParameters(a.value().toInt(), cp);
     }
  }


void XmlToolReader::readAttribute(ToolEntry* t, const QDomAttr& a) {
  qDebug() << "Attribute: " << a.name() << "has value:" << a.value();

  if (a.name() == "Number") t->setNumber(a.value().toInt());
  }


void XmlToolReader::readAttributes(ToolEntry *t, const QDomNamedNodeMap &attrs) {
  int mx = attrs.count();

  for (int i=0; i < mx; ++i) {
      QDomNode n = attrs.item(i);

      if (n.isAttr()) readAttribute(t, n.toAttr());
      }
  }


void XmlToolReader::readAttributes(ToolEntry *t, CuttingParameters* cp, const QDomNamedNodeMap &attrs) {
  int mx = attrs.count();

  for (int i=0; i < mx; ++i) {
      QDomNode n = attrs.item(i);

      if (n.isAttr()) readAttribute(t, cp, n.toAttr());
      }
  }


void XmlToolReader::readElement(ToolEntry* t, CuttingParameters* cp, const QDomElement& e) {
  qDebug() << "Element: " << e.tagName() << " - parent:" << e.parentNode().nodeName();

  if (e.hasAttributes()) readAttributes(t, cp, e.attributes());
  for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
      if (n.isAttr()) readAttribute(t, cp, n.toAttr());
      else if (n.isText()) processText(cp, n);
      else if (n.isElement()) readElement(t, cp, n.toElement());
      else qDebug() << "Element of unknown type!";
      }
  }


void XmlToolReader::readElement(ToolEntry* t, const QDomElement& e) {
  qDebug() << "Element: " << e.tagName() << " - parent:" << e.parentNode().nodeName();

  if (e.hasAttributes()) readAttributes(t, e.attributes());
  for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
      if (n.isAttr()) readAttribute(t, n.toAttr());
      else if (n.isText()) processText(t, n);
      else if (n.isElement()) {
         qDebug() << "node name:" << n.nodeName();

         if (n.nodeName() == "CuttingParameters") {
            CuttingParameters* cp = new CuttingParameters();

            readElement(t, cp, n.toElement());
            }
         else readElement(t, n.toElement());
         }
      else qDebug() << "Element of unknown type!";
      }
  }
