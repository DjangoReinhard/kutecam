/* 
 * **************************************************************************
 * 
 *  file:       projectfile.cpp
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
#include "projectfile.h"
#include <QSettings>
#include <QDebug>


ProjectFile::ProjectFile()
 : tf("SCXXXXXX.prj")
 , cfg(nullptr) {
  tf.open();
  cfg = new QSettings(tf.fileName(), QSettings::IniFormat);
  }


ProjectFile::ProjectFile(const QString& fileName)
 : tf("SCXXXXXX.prj")
 , cfg(nullptr) {
  cfg = new QSettings(fileName, QSettings::IniFormat);
  }


int  ProjectFile::beginReadArray(const QString &prefix) {
  return cfg->beginReadArray(prefix);
  }


void ProjectFile::beginWriteArray(const QString &prefix, int size) {
  cfg->beginWriteArray(prefix, size);
  }


void ProjectFile::beginGroup(const QString& prefix) {
  qDebug() << ">>> === Begin Group >" << prefix << "< ===";

  cfg->beginGroup(prefix);
  }


void ProjectFile::endArray() {
  cfg->endArray();
  }


void ProjectFile::endGroup() {
  qDebug() << "\t<<< ... END Group";

  cfg->endGroup();
  cfg->sync();
  }


QString ProjectFile::fileName() const {
  return cfg->fileName();
  }


void ProjectFile::setArrayIndex(int i) {
  cfg->setArrayIndex(i);
  }


QSettings& ProjectFile::settings() {
  return *cfg;
  }


void ProjectFile::setValue(const QString& key, const QVariant& value) {
  cfg->setValue(key, value);
  }


void ProjectFile::sync() {
  cfg->sync();
  }


QVariant ProjectFile::value(const QString& key, const QVariant& defaultValue) const {
  return cfg->value(key, defaultValue);
  }
