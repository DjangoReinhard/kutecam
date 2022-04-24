/* 
 * **************************************************************************
 * 
 *  file:       work.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    23.4.2022 by Django Reinhard
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
#ifndef WORK_H
#define WORK_H
#include <QObject>
#include <AIS_Shape.hxx>
#include <TopoDS_Face.hxx>


class Work : public QObject
{
  Q_OBJECT
public:
  explicit Work(QObject* parent = nullptr);
  virtual ~Work() = default;

  Handle(AIS_Shape) model;
  std::vector<Handle(AIS_Shape)> modShapes;
  QString           material;
  bool              roundWorkPiece;
//  TopoDS_Face       baseFace;
  Handle(AIS_Shape) workPiece;
  Handle(AIS_Shape) vl;
  Handle(AIS_Shape) vm;
  Handle(AIS_Shape) vr;
  Handle(AIS_Shape) vise;
  Handle(AIS_Shape) modCut;
  Handle(AIS_Shape) wpCut;
  };

#endif // WORK_H
