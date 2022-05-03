/* 
 * **************************************************************************
 * 
 *  file:       operation.cpp
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
#include "operation.h"
#include "targetdefinition.h"
#include "tdfactory.h"
#include "workstep.h"
#include "wsfactory.h"
#include <QSettings>


Operation::Operation(int id, OperationType ot, QObject *parent)
 : QObject(parent)
 , showCutParts(true)
 , showCutPlanes(true)
 , opName(" - NEW - ")
 , opKind(ot)
 , type(CutRoughing)
 , coolingMode(NO_Cooling)
 , dc(NO_Cycle)
 , id(id)
 , cFix(1)
 , curTool(0)
 , outside(true)
 , absolute(true)
 , opA(0)
 , opB(0)
 , opC(0)
 , ae(0)
 , ap(0)
 , fz(0)
 , vc(0)
 , off(0)
 , retZ0(2)
 , retZ1(50)
 , finDepth(0)
 , ret(0)
 , pause(0)
 , wld(0)
 , zMax(0)
 , zMin(0)
 , zTop(0) {
  }


Operation::Operation(int id, QObject *parent)
 : QObject(parent)
 , showCutParts(true)
 , showCutPlanes(true)
 , cutShape(nullptr)
 , opName(" - NEW - ")
 , opKind(0)
 , type(CutRoughing)
 , coolingMode(NO_Cooling)
 , dc(NO_Cycle)
 , id(id)
 , cFix(1)
 , curTool(0)
 , outside(true)
 , absolute(true)
 , opA(0)
 , opB(0)
 , opC(0)
 , ae(0)
 , ap(0)
 , fz(0)
 , vc(0)
 , off(0)
 , retZ0(2)
 , retZ1(50)
 , finDepth(0)
 , ret(0)
 , pause(0)
 , wld(0)
 , zMax(0)
 , zMin(0)
 , zTop(0) {
  }


Operation::Operation(QObject* parent)
 : QObject(parent)
 , showCutParts(true)
 , showCutPlanes(true)
 , cutShape(nullptr)
 , opName(" - NEW - ")
 , opKind(0)
 , type(CutRoughing)
 , coolingMode(NO_Cooling)
 , dc(NO_Cycle)
 , id(0)
 , cFix(1)
 , curTool(0)
 , outside(true)
 , absolute(true)
 , opA(0)
 , opB(0)
 , opC(0)
 , ae(0)
 , ap(0)
 , fz(0)
 , vc(0)
 , off(0)
 , retZ0(2)
 , retZ1(50)
 , finDepth(0)
 , ret(0)
 , pause(0)
 , wld(0)
 , zMax(0)
 , zMin(0)
 , zTop(0) {
  }


int Operation::cooling() const {
  return coolingMode;
  }


double Operation::cutDepth() const {
  return ap;
  }


int Operation::cutType() const {
  return type;
  }


double Operation::cutWidth() const {
  return ae;
  }


int Operation::direction() const {
  return cutDir;
  }


int Operation::drillCycle() const {
  return dc;
  }


double Operation::dwell() const {
  return pause;
  }


double Operation::feedPerTooth() const {
  return fz;
  }


double Operation::finalDepth() const {
  return finDepth;
  }


int Operation::fixture() const {
  return cFix;
  }

#define str(x) #x
#define xstr(x) str(x)

//NOTE: keep in synch with enum "OperationType"
QString Operation::kindAsString() const {
  switch (opKind) {
    case ContourOperation:  return xstr(ContourOperation);
    case ClampingPlugOP:    return xstr(ClampingPlugOP);
    case DrillOperation:    return xstr(DrillOperation);
    case SweepOperation:    return xstr(SweepOperation);
    }
  return QString();
  }


bool Operation::isAbsolute() const {
  return absolute;
  }


bool Operation::isVertical() const {
  return vertical;
  }


gp_Dir& Operation::mainDirection() {
  return opDirection;
  }


gp_Dir Operation::mainDirection() const {
  return opDirection;
  }


double Operation::offset() const {
  return off;
  }


double Operation::operationA() const {
  return opA;
  }


double Operation::operationB() const {
  return opB;
  }

double Operation::operationC() const {
  return opC;
  }


double Operation::qMin() const {
  return ae;
  }


double Operation::qMax() const {
  return ap;
  }


double Operation::speed() const {
  return vc;
  }


bool Operation::isOutside() const {
  return outside;
  }


int Operation::kind() const {
  return opKind;
  }


double Operation::lowerZ() const {
  return zMin;
  }


QString Operation::name() const {
  return opName;
  }


double Operation::retract() const {
  return ret;
  }


double Operation::safeZ0() const {
  return retZ0;
  }


double Operation::safeZ1() const {
  return retZ1;
  }


double Operation::upperZ() const {
  return zMax;
  }


void Operation::setAbsolute(bool absolute) {
  this->absolute = absolute;
  }


void Operation::setCooling(int c) {
  coolingMode = c;
  }


void Operation::setCutDepth(double depth) {
  ap = depth;
  }


void Operation::setCutType(int type) {
  this->type = type;
  }


void Operation::setCutWidth(double width) {
  ae = width;
  }


void Operation::setDirection(int d) {
  cutDir = d;
  }


void Operation::setDrillCycle(int c) {
  dc = c;
  }


void Operation::setFeedPerTooth(double feed) {
  fz = feed;
  }


void Operation::setFinalDepth(double depth) {
  finDepth = depth;
  }


void Operation::setFixture(int fx) {
  cFix = fx;
  }


void Operation::setKind(int kind) {
  opKind = kind;
  }


void Operation::setKind(const QString& kindName) {
  if (!kindName.compare(xstr(ContourOperation))) {
     opKind = ContourOperation;
     }
  else if (!kindName.compare(xstr(ClampingPlugOP))) {
     opKind = ClampingPlugOP;
     }
  else if (!kindName.compare(xstr(DrillOperation))) {
     opKind = DrillOperation;
     }
  else if (!kindName.compare(xstr(SweepOperation))) {
     opKind = SweepOperation;
     }
  }


void Operation::setLowerZ(double z) {
  zMin = z;
  }


void Operation::setName(const QString &name) {
  opName = name;
  }


void Operation::setOffset(double off) {
  this->off = off;
  }


void Operation::setOperationA(double angle) {
  opA = angle;
  }


void Operation::setOperationB(double angle) {
  opB = angle;
  }


void Operation::setOperationC(double angle) {
  opC = angle;
  }


void Operation::setOutside(bool outside) {
  this->outside = outside;
  }


void Operation::setQmin(double q) {
  ae = q;
  }


void Operation::setQmax(double q) {
  ap = q;
  }


void Operation::setSaveZ0(double z) {
  retZ0 = z;
  }


void Operation::setSaveZ1(double z) {
  retZ1 = z;
  }


void Operation::setSpeed(double speed) {
  vc = speed;
  }


void Operation::setToolNum(int num) {
  curTool = num;
  }


void Operation::setTopZ(double z) {
  zTop = z;
  }


void Operation::setUpperZ(double z) {
  zMax = z;
  }


void Operation::setVertical(bool vertical) {
  this->vertical = vertical;
  }


void Operation::setWaterlineDepth(double d) {
  wld = d;
  }


void Operation::restore(QSettings& s) {
  setKind(s.value("opkind").toString());
  setName(s.value("opName").toString());
  setToolNum(s.value("opTool").toInt());
  setOperationA(s.value("opA").toDouble());
  setOperationB(s.value("opB").toDouble());
  setOperationC(s.value("opC").toDouble());
  setAbsolute(s.value("absolute").toBool());
  setCutWidth(s.value("ae").toDouble());
  setCutDepth(s.value("ap").toDouble());
  setDirection(s.value("cutDir").toInt());
  setDrillCycle(s.value("cycle").toInt());
  setCooling(s.value("cooling").toInt());
  setFixture(s.value("fix").toInt());
  setFeedPerTooth(s.value("fz").toDouble());
  setOffset(s.value("off").toDouble());
  setOutside(s.value("outside").toBool());
  setSaveZ0(s.value("R0").toDouble());
  setSaveZ1(s.value("R1").toDouble());
  setSpeed(s.value("vc").toDouble());
  setVertical(s.value("vertical").toBool());
  setWaterlineDepth(s.value("wld").toDouble());
  setFinalDepth(s.value("z").toDouble());
  setLowerZ(s.value("zMin").toDouble());
  setUpperZ(s.value("zMax").toDouble());
  setTopZ(s.value("zTop").toDouble());
  int               mx = s.beginReadArray("Targets");
  TargetDefinition* td;
  Workstep*         ws;

  for (int i=0; i < mx; ++i) {
      s.setArrayIndex(i);
      td = tdFactory->createTargetDefinition(s);
      if (td) targets.push_back(td);
      }
  s.endArray();

  mx = s.beginReadArray("WorkSteps");
  for (int i=0; i < mx; ++i) {
      s.setArrayIndex(i);
      ws = wsFactory->createWorkstep(s);
      if (ws) workingSteps.push_back(ws);
      }
  s.endArray();
  }


void Operation::store(QSettings& s) {
  s.setValue("opkind", kindAsString());
  s.setValue("opName", name());
  s.setValue("opTool", toolNum());
  s.setValue("opA", operationA());
  s.setValue("opB", operationB());
  s.setValue("opC", operationC());
  s.setValue("absolute", isAbsolute());
  s.setValue("ae", cutWidth());
  s.setValue("ap", cutDepth());
  s.setValue("cutDir", direction());
  s.setValue("cycle", drillCycle());
  s.setValue("cooling", cooling());
  s.setValue("fix", fixture());
  s.setValue("fz", feedPerTooth());
  s.setValue("off", offset());
  s.setValue("outside", isOutside());
  s.setValue("R0", safeZ0());
  s.setValue("R1", safeZ1());
  s.setValue("vc", speed());
  s.setValue("vertical", isVertical());
  s.setValue("wld", waterlineDepth());
  s.setValue("z", finalDepth());
  s.setValue("zMin", lowerZ());
  s.setValue("zMax", upperZ());
  s.setValue("zTop", topZ());

  s.beginWriteArray("Targets");
  for (int i=0; i < targets.size(); ++i) {
      s.setArrayIndex(i);
      targets.at(i)->store(s);
      }
  s.endArray();

  s.beginWriteArray("WorkSteps");
  for (int i=0; i < workingSteps.size(); ++i) {
      s.setArrayIndex(i);
      workingSteps.at(i)->store(s);
      }
  s.endArray();
  }


int Operation::toolNum() const {
  return curTool;
  }


double Operation::topZ() const {
  return zTop;
  }


QString Operation::toString() const {
  return opName;
  }


double Operation::waterlineDepth() const {
  return wld;
  }


std::vector<Workstep*> Operation::workSteps() const {
  return workingSteps;
  }


std::vector<Workstep*>& Operation::workSteps() {
  return workingSteps;
  }


//QString    Operation::OPSweep        = tr("Sweep");
//QString    Operation::OPContour      = tr("Contour");
//QString    Operation::OPClampingPlug = tr("Contour");
//QString    Operation::OPDrill        = tr("Drill");
QString    Operation::OTRoughing     = tr("Roughing");
QString    Operation::OTFinish       = tr("Finish");
TDFactory* Operation::tdFactory      = nullptr;
WSFactory* Operation::wsFactory      = nullptr;
