/*
 * **************************************************************************
 *
 *  file:       clipdialog.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    26.5.2022 by Django Reinhard
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
#ifndef CLIPDIALOG_H
#define CLIPDIALOG_H
#include <QDialog>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
QT_BEGIN_NAMESPACE
namespace Ui {
class ClipDialog;
}
QT_END_NAMESPACE
class QShowEvent;


class ClipDialog : public QDialog
{
  Q_OBJECT
public:
  ClipDialog(const gp_Pnt& pos, const gp_Dir& dir, QWidget* parent = nullptr);

  bool   clippingEnabled() const;
  gp_Pnt clipPosition()  const;
  gp_Dir clipDirection() const;

protected:
  void showEvent(QShowEvent *event);

private:
  Ui::ClipDialog* ui;
  };
#endif // CLIPDIALOG_H
