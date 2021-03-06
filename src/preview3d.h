/* 
 * **************************************************************************
 * 
 *  file:       preview3d.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements                        
 *  created:    11.4.2022 by Django Reinhard
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
#ifndef PREVIEW3D_H
#define PREVIEW3D_H
#include <QWidget>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
QT_BEGIN_NAMESPACE
namespace Ui {
class Preview3D;
}
QT_END_NAMESPACE
class OcctQtViewer;
class QPushButton;


class Preview3D : public QWidget
{
  Q_OBJECT
public:
  explicit Preview3D(QWidget *parent = nullptr);
  virtual ~Preview3D();

  OcctQtViewer* viewer3D() const { return view3D; }

public slots:
  void toolHint(bool showHint);

protected slots:
  void toggleHide();
  void toggleClip(bool hitX);
  void toggleWireframe();

private:
  Ui::Preview3D* ui;
  OcctQtViewer*  view3D;
  gp_Pnt         clipPos;
  gp_Dir         clipDir;
  };
#endif // PREVIEW3D_H
