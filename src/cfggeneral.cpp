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
#include "cfggeneral.h"
#include "configpage.h"
#include "ui_cfgGeneral.h"
#include "core.h"
#include <QSortFilterProxyModel>
#include <QDebug>


CfgGeneral::CfgGeneral(QWidget *parent)
 : QWidget(parent)
 , ui(new Ui::GeneralConfig) {
  ui->setupUi(this);
  master = static_cast<ConfigPage*>(parent);
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
  ui->cAutoRotateSelection->setChecked(Core().autoRotateSelection());

  for (int i=0; i < Core().ppModel()->rowCount(); ++i) {
      QModelIndex mi    = Core().ppModel()->index(i, 0);
      QString     entry = Core().ppModel()->data(mi).toString();

      if (!entry.compare(Core().postProcessor())) {
         ui->cbPostProcessors->setCurrentIndex(ppProxy->mapFromSource(mi).row());
         break;
         }
      }
  switch (Core().machineType()) {
    case 1: ui->rbAC->setChecked(true);  break;
    case 2: ui->rbBC->setChecked(true);  break;
    case 3: ui->rbABC->setChecked(true); break;
    }
  connect(ui->cbPostProcessors, &QComboBox::currentTextChanged, this, [=]{ Core().setPostProcessor(ui->cbPostProcessors->currentText()); });
  connect(ui->cAllInOne, &QCheckBox::toggled, this, &CfgGeneral::allInOneToggled);
  connect(ui->cSepToolChange, &QCheckBox::toggled, this, [=]{ Core().setSepWithToolChange(ui->cSepToolChange->isChecked()); });
  connect(ui->cAutoRotateSelection, &QCheckBox::toggled, this, &CfgGeneral::autoRotatedChanged);
  connect(ui->rbAC,  &QRadioButton::clicked, this, &CfgGeneral::updateMachineType);
  connect(ui->rbBC,  &QRadioButton::clicked, this, &CfgGeneral::updateMachineType);
  connect(ui->rbABC, &QRadioButton::clicked, this, &CfgGeneral::updateMachineType);
  connect(ui->cAisTable, &QCheckBox::toggled, this, [=]{ Core().setAAxisIsTable(ui->cAisTable->isChecked()); });
  connect(ui->cBisTable, &QCheckBox::toggled, this, [=]{ Core().setBAxisIsTable(ui->cBisTable->isChecked()); });
  connect(ui->cCisTable, &QCheckBox::toggled, this, [=]{ Core().setCAxisIsTable(ui->cCisTable->isChecked()); });
  connect(master, &ConfigPage::machineTypeChanged, this, &CfgGeneral::handleMachineType);

  allInOneToggled(ui->cAllInOne->isChecked());
  }


void CfgGeneral::allInOneToggled(const QVariant& v) {
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


void CfgGeneral::autoRotatedChanged() {
  Core().setAutoRotateSelection(ui->cAutoRotateSelection->isChecked());
  }


void CfgGeneral::handleMachineType(int machineType) {
  Core().setMachineType(machineType);

  qDebug() << "CG::machine-type: " << machineType;
  switch (machineType) {
    case 1:
         ui->cAisTable->setEnabled(true);
         ui->cBisTable->setEnabled(false);
         break;
    case 2:
         ui->cAisTable->setEnabled(false);
         ui->cBisTable->setEnabled(true);
         break;
    case 3:
         ui->cAisTable->setEnabled(true);
         ui->cBisTable->setEnabled(true);
         break;
    }
  }


void CfgGeneral::updateMachineType() {
  if (ui->rbAC->isChecked())       emit master->machineTypeChanged(1);
  else if (ui->rbBC->isChecked())  emit master->machineTypeChanged(2);
  else if (ui->rbABC->isChecked()) emit master->machineTypeChanged(3);
  }
