/* 
 * **************************************************************************
 * 
 *  file:       cfggcode.cpp
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
#include "cfggcode.h"
#include "ui_cfgGCode.h"
#include "core.h"
#include <QSortFilterProxyModel>


CfgGCode::CfgGCode(QWidget *parent)
 : QWidget(parent)
 , ui(new Ui::GCodeConfig) {
  ui->setupUi(this);

  ui->cAllInOne->setChecked(Core().isAllInOneOperation());
  ui->cSepToolChange->setChecked(Core().isSepWithToolChange());
  ui->cAisTable->setChecked(Core().isAAxisTable());
  ui->cBisTable->setChecked(Core().isBAxisTable());
  ui->cCisTable->setChecked(Core().isCAxisTable());

  ppProxy = new QSortFilterProxyModel(this);
  ppProxy->setSourceModel(Core().ppModel());
  ppProxy->setDynamicSortFilter(true);
  ppProxy->sort(0);
  ui->cbPostProcessors->setModel(ppProxy);

//  QAbstractItemModel* m = ui->cbPostProcessors->model();

  for (int i=0; i < Core().ppModel()->rowCount(); ++i) {
      QModelIndex mi    = Core().ppModel()->index(i, 0);
      QString     entry = Core().ppModel()->data(mi).toString();

      if (!entry.compare(Core().postProcessor())) {
         ui->cbPostProcessors->setCurrentIndex(ppProxy->mapFromSource(mi).row());
         break;
         }
      }
  connect(ui->cbPostProcessors, &QComboBox::currentTextChanged, this, [=]{ Core().setPostProcessor(ui->cbPostProcessors->currentText()); });
  connect(ui->cAllInOne, &QCheckBox::toggled, this, &CfgGCode::allInOneToggled);
  connect(ui->cSepToolChange, &QCheckBox::toggled, this, [=]{ Core().setSepWithToolChange(ui->cSepToolChange->isChecked()); });
  connect(ui->cAisTable, &QCheckBox::toggled, this, [=]{ Core().setAAxisIsTable(ui->cAisTable->isChecked()); });
  connect(ui->cBisTable, &QCheckBox::toggled, this, [=]{ Core().setBAxisIsTable(ui->cBisTable->isChecked()); });
  connect(ui->cCisTable, &QCheckBox::toggled, this, [=]{ Core().setCAxisIsTable(ui->cCisTable->isChecked()); });

  allInOneToggled(ui->cAllInOne->isChecked());
  }


void CfgGCode::allInOneToggled(const QVariant& v) {
  if (v.toBool()) {
     ui->cSepToolChange->setChecked(false);
     ui->cSepToolChange->setEnabled(false);
     }
  else {
     ui->cSepToolChange->setEnabled(true);
     }
  Core().setAllInOneOperation(ui->cAllInOne->isChecked());
  Core().setSepWithToolChange(ui->cSepToolChange->isChecked());
  }
