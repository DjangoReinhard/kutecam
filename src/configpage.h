/*
 * **************************************************************************
 *
 *  file:       configpage.h
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
#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H
#include "applicationwindow.h"
QT_END_NAMESPACE
class StringListModel;
class ToolListModel;
class QCloseEvent;
class QToolBox;
class ViseListModel;


class ConfigPage : public ApplicationWindow
{
  Q_OBJECT
public:
  explicit ConfigPage(StringListModel* matModel, ViseListModel* vises, QWidget *parent = nullptr);

  void initialize();

public slots:
  void closeEvent(QCloseEvent* e) override;
  void loadTools();
  void saveTools();

private:
  QToolBox*        pages;
  ToolListModel*   tools;
  StringListModel* matModel;
  ViseListModel*   vises;
  };
#endif // CONFIGPAGE_H
