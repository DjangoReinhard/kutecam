/* 
 * **************************************************************************
 * 
 *  file:       operation.h
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
#ifndef OPERATION_H
#define OPERATION_H
#include <QObject>
#include <QString>
#include "DrillCycle.h"
#include <AIS_Shape.hxx>
#include <Bnd_Box.hxx>
#include <TopoDS_Edge.hxx>
#include <set>
class GOContour;
class QSettings;
class TargetDefinition;
class TDFactory;
class ToolEntry;
class TestRunner;
class Workstep;
class WSFactory;


enum OperationType
{
  ContourOperation
, DrillOperation
, SweepOperation
, ClampingPlugOP
  };


enum CutType
{
  CutRoughing
, CutFinish
  };


enum CutDirection
{
  ParallelFeed
, AgainstFeed
, IgnoreFeed
  };


enum Cooling
{
  NO_Cooling
, MistCooling
, FloodCooling
  };


class Operation : public QObject
{
  Q_OBJECT
public:
//  static QString OPSweep;
//  static QString OPContour;
//  static QString OPClampingPlug;
//  static QString OPDrill;
  static QString OTRoughing;
  static QString OTFinish;
  explicit Operation(int id, OperationType ot, QObject* parent = nullptr);
  explicit Operation(int id, QObject* parent = nullptr);

  int           cooling() const;
  double        cutDepth() const;
  int           cutType() const;
  double        cutWidth() const;
  int           direction() const;
  int           drillCycle() const;
  double        dwell() const;
  double        feedPerTooth() const;
  double        finalDepth() const;
  int           fixture() const;
  bool          isAbsolute() const;
  bool          isOutside() const;
  bool          isVertical() const;
  int           kind() const;
  QString       kindAsString() const;
  double        lowerZ() const;
  gp_Dir&       mainDirection();
  gp_Dir        mainDirection() const;
  QString       name() const;
  double        offset() const;
  double        operationA() const;
  double        operationB() const;
  double        operationC() const;
  double        qMin() const;
  double        qMax() const;
  double        retract() const;
  double        safeZ0() const;
  double        safeZ1() const;
  double        speed() const;
  int           toolNum() const;
  QString       toString() const;
  double        topZ() const;
  double        upperZ() const;
  double        waterlineDepth() const;
  std::vector<Workstep*>  workSteps() const;
  std::vector<Workstep*>& workSteps();


  void    setAbsolute(bool absolute);
  void    setCooling(int c);
  void    setCutDepth(double depth);
  void    setCutType(int ct);
  void    setCutWidth(double width);
  void    setDirection(int d);
  void    setDrillCycle(int c);
  void    setDwell(double pauseSecs);
  void    setFeedPerTooth(double feed);
  void    setFinalDepth(double depth);
  void    setFixture(int fx);
  void    setKind(int ot);
  void    setKind(const QString& kindName);
  void    setLowerZ(double z);
  void    setName(const QString& name);
  void    setOffset(double off);
  void    setOperationA(double angle);
  void    setOperationB(double angle);
  void    setOperationC(double angle);
  void    setOutside(bool outside);
  void    setQmin(double q);
  void    setQmax(double q);
  void    setRetract(double r);
  void    setSaveZ0(double z);
  void    setSaveZ1(double z);
  void    setSpeed(double speed);
  void    setToolNum(int num);
  void    setTopZ(double z);
  void    setUpperZ(double z);
  void    setVertical(bool vertical);
  void    setWaterlineDepth(double d);

  void    store(QSettings& settings);

  Handle(AIS_Shape)              workPiece;
  Handle(AIS_Shape)              drill;
  Bnd_Box                        wpBounds;
  Bnd_Box                        mBounds;
  Bnd_Box                        shBounds;
  Bnd_Box                        vBounds;
  Bnd_Box                        dBounds;
  std::vector<Handle(AIS_Shape)> cShapes;
  std::vector<TargetDefinition*> targets;
  QVector<Handle(AIS_Shape)>     toolPaths;
  Handle(AIS_Shape)              cutPart;
  GOContour*                     cutShape;
  bool                           showCutPlanes;
  bool                           showCutParts;
  static TDFactory*              tdFactory;
  static WSFactory*              wsFactory;

private:
  explicit Operation(QObject* parent = nullptr);

  void restore(QSettings& settings);

  QString                   opName;
  gp_Dir                    opDirection;
  int /* OperationType */   opKind;
  int /* CutType */         type;
  int /* CutDirection */    cutDir;
  int /* Cooling */         coolingMode;
  int /* drillCycle */      dc;
  int                       id;
  int                       cFix;
  int                       curTool;
  bool                      outside;
  bool                      absolute;
  bool                      vertical;
  double                    opA;
  double                    opB;
  double                    opC;
  double                    ae;
  double                    ap;
  double                    fz;
  double                    vc;
  double                    off;
  double                    retZ0;
  double                    retZ1;
  double                    finDepth;
  double                    ret;
  double                    pause;
  double                    wld;
  double                    zMax;
  double                    zMin;
  double                    zTop;
  std::vector<Workstep*>    workingSteps;
  std::vector<TopoDS_Edge>  modEdges;
  std::vector<TopoDS_Edge>  wpEdges;
  friend class Kernel;
  friend class TestRunner;
  };
#endif // OPERATION_H
