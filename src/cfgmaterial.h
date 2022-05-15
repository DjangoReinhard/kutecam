/* 
 * **************************************************************************
 * 
 *  file:       cfgmaterial.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    15.5.2022 by Django Reinhard
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
#ifndef CFGMATERIAL_H
#define CFGMATERIAL_H
#include <QWidget>
QT_BEGIN_NAMESPACE
namespace Ui {
class MaterialConfig;
}
QT_END_NAMESPACE
class StringListModel;
class QItemSelection;
class QSortFilterProxyModel;


class CfgMaterial : public QWidget
{
  Q_OBJECT
public:
  explicit CfgMaterial(StringListModel* matModel, QWidget *parent = nullptr);

public slots:
  void addMaterial();
  void delMaterial();
  void materialSelected(const QItemSelection &selected, const QItemSelection &deselected);
  bool eventFilter(QObject* o, QEvent* event) override;

private:
  Ui::MaterialConfig*    ui;
  StringListModel*       matModel;
  QSortFilterProxyModel* matProxy;
  };
#endif // CFGMATERIAL_H
