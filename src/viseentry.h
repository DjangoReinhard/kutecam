/* 
 * **************************************************************************
 * 
 *  file:       viseentry.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    20.3.2022 by Django Reinhard
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
#ifndef VISEENTRY_H
#define VISEENTRY_H
#include <QObject>


class ViseEntry : public QObject
{
  Q_OBJECT
public:
  explicit ViseEntry(QObject *parent = nullptr);
  explicit ViseEntry(const QString& name, const QString& left, const QString& right, const QString& middle = QString(), QObject* parent = nullptr);

  QString name() const;
  QString left() const;
  QString middle() const;
  QString right() const;

  void setName(const QString& name);
  void setLeft(const QString& left);
  void setMiddle(const QString& middle);
  void setRight(const QString& right);

private:
  QString vName;
  QString vLeft;
  QString vMiddle;
  QString vRight;
  };
#endif // VISEENTRY_H
