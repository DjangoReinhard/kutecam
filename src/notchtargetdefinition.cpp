/* 
 * **************************************************************************
 * 
 *  file:       notchtargetdefinition.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    21.6.2022 by Django Reinhard
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
#include "notchtargetdefinition.h"
#include <QSettings>


NotchTargetDefinition::NotchTargetDefinition(gp_Pln face, const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, QObject *parent)
 : TargetDefinition(face.Location(), 0, parent)
 , btmFace(face) {
  bp[0] = p0;
  bp[1] = p1;
  bp[2] = p2;
  bp[3] = p3;
  }


NotchTargetDefinition::NotchTargetDefinition(QSettings& s, QObject* parent)
 : TargetDefinition(s, parent) {
  gp_Pnt dir;

  dir.SetX(s.value("tdDirX").toDouble());
  dir.SetY(s.value("tdDirY").toDouble());
  dir.SetZ(s.value("tdDirZ").toDouble());
  btmFace = gp_Pln(pos(), { dir.X(), dir.Y(), dir.Z() });
  bp[0].SetX(s.value("tdP0X").toDouble());
  bp[0].SetY(s.value("tdP0Y").toDouble());
  bp[0].SetZ(s.value("tdP0Z").toDouble());
  bp[1].SetX(s.value("tdP1X").toDouble());
  bp[1].SetY(s.value("tdP1Y").toDouble());
  bp[1].SetZ(s.value("tdP1Z").toDouble());
  bp[2].SetX(s.value("tdP2X").toDouble());
  bp[2].SetY(s.value("tdP2Y").toDouble());
  bp[2].SetZ(s.value("tdP2Z").toDouble());
  bp[3].SetX(s.value("tdP3X").toDouble());
  bp[3].SetY(s.value("tdP3Y").toDouble());
  bp[3].SetZ(s.value("tdP3Z").toDouble());
  }


void NotchTargetDefinition::store(QSettings &s) {
  s.setValue("tdType", "NotchTarget");
  s.setValue("tdDirX", btmFace.Axis().Direction().X());
  s.setValue("tdDirY", btmFace.Axis().Direction().Y());
  s.setValue("tdDirZ", btmFace.Axis().Direction().Z());
  s.setValue("tdP0X", bp[0].X());
  s.setValue("tdP0Y", bp[0].Y());
  s.setValue("tdP0Z", bp[0].Z());
  s.setValue("tdP1X", bp[1].X());
  s.setValue("tdP1Y", bp[1].Y());
  s.setValue("tdP1Z", bp[1].Z());
  s.setValue("tdP2X", bp[2].X());
  s.setValue("tdP2Y", bp[2].Y());
  s.setValue("tdP2Z", bp[2].Z());
  s.setValue("tdP3X", bp[3].X());
  s.setValue("tdP3Y", bp[3].Y());
  s.setValue("tdP3Z", bp[3].Z());
  }


gp_Pnt NotchTargetDefinition::borderPoint(int index) const {
  if (index > 3) return gp_Pnt();
  return bp[index];
  }


gp_Pln NotchTargetDefinition::bottom() const {
  return btmFace;
  }


QString NotchTargetDefinition::toString() const {
  QString rv = QString("Face: %1/%2/%3"
                       "\nB0\t(%4/%5/%6 - %7/%8/%9)"
                       "\nB0\t(%10/%11/%12 - %13/%14/%15)")
                  .arg(btmFace.Location().X(), 0, 'f', 3)
                  .arg(btmFace.Location().Y(), 0, 'f', 3)
                  .arg(btmFace.Location().Z(), 0, 'f', 3)
                  .arg(bp[0].X(), 0, 'f', 3)
                  .arg(bp[0].Y(), 0, 'f', 3)
                  .arg(bp[0].Z(), 0, 'f', 3)
                  .arg(bp[1].X(), 0, 'f', 3)
                  .arg(bp[1].Y(), 0, 'f', 3)
                  .arg(bp[1].Z(), 0, 'f', 3)
                  .arg(bp[2].X(), 0, 'f', 3)
                  .arg(bp[2].Y(), 0, 'f', 3)
                  .arg(bp[2].Z(), 0, 'f', 3)
                  .arg(bp[3].X(), 0, 'f', 3)
                  .arg(bp[3].Y(), 0, 'f', 3)
                  .arg(bp[3].Z(), 0, 'f', 3)
                  ;
  return rv;
  }
