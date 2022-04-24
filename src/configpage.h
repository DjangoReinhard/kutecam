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
#include <AIS_Shape.hxx>
QT_BEGIN_NAMESPACE
namespace Ui {
class MiscPage;
}
QT_END_NAMESPACE
class StringListModel;
class ViseListModel;
class QEvent;
class QItemSelection;
class QShowEvent;
class QStringListModel;


class ConfigPage : public ApplicationWindow
{
  Q_OBJECT
public:
  explicit ConfigPage(StringListModel* matModel, ViseListModel* vModel, QWidget *parent = nullptr);


public slots:
  void addMaterial();
  void addVise();
  void closeEvent(QCloseEvent* e) override;
  bool eventFilter(QObject* o, QEvent* event) override;
  void delMaterial();
  void delVise();
  void setLeftVise();
  void setMidVise();
  void setRightVise();
  void showEvent(QShowEvent* event) override;
  void materialSelected(const QItemSelection &selected, const QItemSelection &deselected);
  void viseSelected(const QItemSelection &selected, const QItemSelection &deselected);

private:
  Ui::MiscPage*     ui;
  ViseListModel*    vModel;
  StringListModel*  matModel;
  Handle(AIS_Shape) leftVise;
  Handle(AIS_Shape) midVise;
  Handle(AIS_Shape) rightVise;
  };
#endif // CONFIGPAGE_H
