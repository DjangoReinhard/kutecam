/* 
 * **************************************************************************
 * 
 *  file:       aboutdialog.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements
 *  created:    22.1.2022 by Django Reinhard
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
#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H
#include <QDialog>

class QDialogButtonBox;
class QTabWidget;
class QListView;


class GeneralTab : public QWidget
{
  Q_OBJECT
public:
  explicit GeneralTab(QWidget* parent = nullptr);
  };


class GLInfoTab : public QWidget
{
  Q_OBJECT
public:
  explicit GLInfoTab(QWidget* parent = nullptr);
  };


class PluginsTab : public QWidget
{
  Q_OBJECT
public:
  explicit PluginsTab(QWidget* parent = nullptr);

private:
  QListView* list;
  };


class AboutDialog : public QDialog
{
  Q_OBJECT
public:
  AboutDialog(QWidget* parent = nullptr);

private:
  QTabWidget*       tw;
  QDialogButtonBox* bb;
  };
#endif // ABOUTDIALOG_H
