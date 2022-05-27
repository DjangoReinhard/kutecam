/* 
 * **************************************************************************
 * 
 *  file:       toolentry.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    4.4.2022 by Django Reinhard
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
#ifndef TOOLENTRY_H
#define TOOLENTRY_H
#include <QObject>
#include <QMap>
class CuttingParameters;


class ToolEntry : public QObject
{
  Q_OBJECT
public:
  explicit ToolEntry(int num, const QString& name = QString(), QObject* parent = nullptr);
  explicit ToolEntry(const ToolEntry& other);

  ToolEntry& operator=(const ToolEntry& other);

  void    dump() const;
  int     collet() const;
  double  tipDiameter() const;
  int     toolNumber() const;
  QString toolName() const;
  double  fluteDiameter() const;
  double  fluteLength() const;
  double  cuttingAngle() const;
  double  cuttingDepth() const;
  double  shankDiameter() const;
  double  freeLength() const;
  bool    hasMaterial(const QString& material) const;
  int     numFlutes() const;
  void    setCuttingAngle(double ang);
  void    setCollet(int collet);
  void    setFluteDiameter(double diam);
  void    setFluteLength(double len);
  void    setCuttingDepth(double depth);
  void    setShankDiameter(double diam);
  void    setFreeLength(double len);
  void    setTipDiameter(double diam);
  void    setToolName(const QString& name);
  void    setNumFlutes(int num);
  QString toString() const;
  CuttingParameters* cutParameter(int row);
  CuttingParameters* cutParameter(const QString& material);
  QVector<CuttingParameters*>&       cutParameters();
  const QVector<CuttingParameters*>& cutParameters() const;

private:
  explicit ToolEntry(QObject* parent = nullptr);
  void     setNumber(int number);
  void     insertCuttingParameters(int row, CuttingParameters* cp);

  int     number;
  int     cColl;
  QString name;
  double  tipDiam;
  double  flDiam;
  double  flLen;
  double  cutDepth;
  double  cutAngle;
  double  shankDiam;
  double  freeLen;
  int     cFlutes;
  QVector<CuttingParameters*> cutParams;

  friend class XmlToolReader;
  };
#endif // TOOLENTRY_H
