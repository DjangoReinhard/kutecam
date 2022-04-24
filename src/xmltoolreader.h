/* 
 * **************************************************************************
 * 
 *  file:       xmltoolreader.h
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
#ifndef XMLTOOLREADER_H
#define XMLTOOLREADER_H
#include <QObject>
class QIODevice;
class QDomAttr;
class QDomDocument;
class QDomElement;
class QDomNamedNodeMap;
class QDomNode;
class CuttingParameters;
class ToolEntry;


class XmlToolReader : public QObject
{
  Q_OBJECT
public:
  explicit XmlToolReader(QObject* parent = nullptr);

  QVector<ToolEntry*> read(QIODevice* io);
  QVector<ToolEntry*> read(const QByteArray& ba);

protected:
  void processDoc(const QDomDocument& doc, QVector<ToolEntry*>& tools);
  void processText(CuttingParameters* cp, const QDomNode& n);
  void processText(ToolEntry* t, const QDomNode& n);
  void readAttribute(ToolEntry* t, const QDomAttr& a);
  void readAttribute(ToolEntry* t, CuttingParameters* cp, const QDomAttr& a);
  void readAttributes(ToolEntry* t, const QDomNamedNodeMap& attrs);
  void readAttributes(ToolEntry* t, CuttingParameters* cp, const QDomNamedNodeMap& attrs);
  void readElement(ToolEntry* t, const QDomElement& e);
  void readElement(ToolEntry* t, CuttingParameters* cp, const QDomElement& e);
  };
#endif // XMLTOOLREADER_H
