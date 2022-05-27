/* 
 * **************************************************************************
 * 
 *  file:       selectioninfohandler.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    26.5.2022 by Django Reinhard
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
#ifndef SELECTIONINFOHANDLER_H
#define SELECTIONINFOHANDLER_H
#include <QObject>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <Geom_Curve.hxx>


class SelectionInfoHandler : public QObject
{
  Q_OBJECT
public:
  explicit SelectionInfoHandler(QObject *parent = nullptr);

  void evalSelection();

//protected:
  void exploreCurve(Handle(Geom_Curve) c, double first, double last);
  void exploreEdge(TopoDS_Edge e);
  void exploreFace(TopoDS_Face f);
  };
#endif // SELECTIONINFOHANDLER_H
