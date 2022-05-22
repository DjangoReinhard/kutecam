/* 
 * **************************************************************************
 * 
 *  file:       ppSinumeric840D.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    17.5.2022 by Django Reinhard
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
#include "ppSinumeric840D.h"
#include "DrillCycle.h"
#include "gp_Pnt.hxx"
#include <QString>
const double MinDelta = 1e-5;
const int    Decimals = 3;


PPSinumeric840D::PPSinumeric840D(QObject* parent)
 : DINPostProcessor(parent) {
  }


QString PPSinumeric840D::fixtureID(int f) {
  return QString("G%1").arg(f < 5 ? 53 + f : 500 + f);
  }


QString PPSinumeric840D::genDefineCycle(int c, double topZ, double r0, double r1, double depth, double qMin, double qMax, double retract, double dwell, int feed) {
  QString cmd("MCALL CYCLE");

  switch (c) {
    case FineBoringCycle: // G76 (wohlhaupter)
//         return QString("G76 Z%1 R%2 Q%3 P%4 F%5").arg(finalZ, 0, 'f', Decimals)
//                                              .arg(r0, 0, 'f', Decimals)
//                                              .arg(retract, 0, 'f', Decimals)
//                                              .arg(dwell, 0, 'f', 0)
//                                              .arg(feed);
         break;
    case SpotDrillCycle:  // G81  - Parameters: 0 rtp  (retract plane r1 [absolute])
                          //                    1 rfp  (reference plane topZ [absolute])
                          //                    2 sdis (safe distance r0)
                          //                    3 dp   (drill depth absolute)
                          //                    4 dpr  (drill depth relative)
                          //
         cmd += QString("81(%1, %2, %3, , %4)")
                       .arg(topZ + r1, 0, 'f', Decimals)
                       .arg(topZ, 0, 'f', Decimals)
                       .arg(r0, 0, 'f', Decimals)
                       .arg(topZ - depth, 0, 'f', Decimals)
                       ;
         break;
    case DrillWithDwell:  // G82  - Parameters: 0 rtp   (retract plane r1)
                          //                    1 rfp   (reference plane topZ)
                          //                    2 sdis  (safe distance r0)
                          //                    3 dp    (drill depth absolute)
                          //                    4 dpr   (drill depth relative)
                          //                    5 dwell (pause at drill depth)
                          //
         cmd += QString("82(%1, %2, %3, , %4, %5)")
                       .arg(topZ + r1, 0, 'f', Decimals)
                       .arg(topZ, 0, 'f', Decimals)
                       .arg(r0, 0, 'f', Decimals)
                       .arg(topZ - depth, 0, 'f', Decimals)
                       .arg(dwell, 0, 'f', Decimals)
                       ;
         break;
    case PeckDrilling:    // G83  - Parameters: 0 rtp     - retract plane
                          //                    1 rfp     - reference plane
                          //                    2 sdis    - safe distance
                          //                    3 dp      - drill depth (abs)
                          //                    4 dpr     - drill depth (rel)
                          //                    5 fdep    - first drill depth (abs)
                          //                    6 fdpr    - first drill depth (rel - without sign)
                          //                    7 dam     - degression
                          //                    8 dtb     - dwell unit at drill depth (> 0 time, < 0 turns)
                          //                    9 dts     - dwell unit at start       (> 0 time, < 0 turns)
                          //                   10 frf     - feed factor (0.001 - 1)
                          //                   11 vari    - 0 break / 1 retract
                          //                   12 _axn    - spindle axis
                          //                   13 _mdep   - qmin
                          //                   14 _vrt    - retract value
                          //                   15 _dtd    - dwell at drill depth
                          //                   16 _dis1   - gap at drill depth
                          //
         cmd += QString("81(%1, %2, %3,, %4,, %5, %6, 1, 1, 1, 1,, %7,, 0, 0)")
                       .arg(topZ + r1, 0, 'f', Decimals)
                       .arg(topZ, 0, 'f', Decimals)
                       .arg(r0, 0, 'f', Decimals)
                       .arg(topZ - depth, 0, 'f', Decimals)
                       .arg(qMax, 0, 'f', Decimals)
                       .arg(qMax - qMin, 0, 'f', Decimals)
                       .arg(qMin, 0, 'f', Decimals)
                       ;
         break;
    case Tapping:         // G84  - Parameters: 0 rtp
                          //                    1 rfp     - reference plane
                          //                    2 sdis    - safe distance
                          //                    3 dp      - drill depth (abs)
                          //                    4 dpr     - drill depth (rel)
                          //                    5 dtb     - dwell unit at drill depth (> 0 time, < 0 turns)
                          //                    6 sdac    - direction after cycle (3/4/5)
                          //                    7 mpit    - metric thread size (3 - 48 [M3 - M48])
                          //                                sign determines threading direction
                          //                    8 pit     - thread height (0.001 - 2000mm)
                          //                                sign determines threading direction
                          //                    9 poss    - orientation for spindle stop (in degree)
                          //                   10 sst     - speed for tapping
                          //                   11 sst1    - speed for retract
                          //                   12 _axn    - spindle axis
                          //                   13 _ptab   - thread height unit 0 - according G10/G20
                          //                                                   1 - height in mm
                          //                                                   2 - height in turns per inch
                          //                                                   3 - height in inch/turn
                          //                   14 _techno
                          //                   15 _vari
                          //                   16 _dam
                          //                   17 _vrt
                          //                    )
         cmd += QString("84(%1, %2, %3,, %4,, 3,,   %5, 90, 100)")
                       .arg(topZ + r1, 0, 'f', Decimals)
                       .arg(topZ, 0, 'f', Decimals)
                       .arg(r0, 0, 'f', Decimals)
                       .arg(topZ - depth, 0, 'f', Decimals)
                       .arg((double)feed / 100.0, 0, 'f', Decimals)
                       ;
         break;
    case BoringCycle:     // G85  - Parameters: 0 rtp     - retract plane
                          //                    1 rfp     - reference plane
                          //                    2 sdis    - safe distance
                          //                    3 dp      - drill depth (abs)
                          //                    4 dpr     - drill depth (rel)
                          //                    5 dtb     - dwell unit at drill depth (> 0 time, < 0 turns)
                          //                    6 ffr     - feed
                          //                    7 rff     - feed for retract
                          //                      )
         cmd += QString("85(%1, %2, %3,, %4,, %5")
                       .arg(topZ + r1, 0, 'f', Decimals)
                       .arg(topZ, 0, 'f', Decimals)
                       .arg(r0, 0, 'f', Decimals)
                       .arg(topZ - depth, 0, 'f', Decimals)
                       .arg((double)feed / 100.0, 0, 'f', Decimals)
                       .arg(feed);
         break;
    default: break;
    }
  return cmd;
  }


QString PPSinumeric840D::genEndCycle() {
  return "MCALL";
  }


QString PPSinumeric840D::getFileExtension() const {
  return "arc";
  }


QString PPSinumeric840D::genLengthCorrEnd() {
  return QString();
  }


QString PPSinumeric840D::genLengthCorrStart(int) {
  return QString();
  }


QString PPSinumeric840D::genPrepareTool(int toolNum) {
  return QString("T%1").arg(toolNum);
  }


QString PPSinumeric840D::genToolChange() {
  return "M6 D1";
  }
