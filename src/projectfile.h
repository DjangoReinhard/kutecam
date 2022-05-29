/* 
 * **************************************************************************
 * 
 *  file:       projectfile.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    7.4.2022 by Django Reinhard
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
#ifndef PROJECTFILE_H
#define PROJECTFILE_H
#include <QTemporaryFile>
#include <QVariant>
class QString;
class QSettings;


class ProjectFile
{
public:
  explicit ProjectFile();
  explicit ProjectFile(const QString& fileName);
  virtual ~ProjectFile();

  void       beginGroup(const QString& prefix);
  int        beginReadArray(const QString &prefix);
  void       beginWriteArray(const QString &prefix, int size = -1);
  void       endArray();
  void       endGroup();
  QString    fileName() const;
  void       remove(const QString& key);
  QSettings& settings();
  void       setArrayIndex(int i);
  void       setValue(const QString& key, const QVariant& value);
  void       sync();
  QString    tempFileName() const;
  QVariant   value(const QString& key, const QVariant& defaultValue = QVariant()) const;

private:
  QTemporaryFile tf;
  QSettings*     cfg;
  };
#endif // PROJECTFILE_H
