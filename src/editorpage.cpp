/* 
 * **************************************************************************
 * 
 *  file:       editorpage.cpp
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create gcode for toolpaths created from CAD models
 *  created:    22.5.2022 by Django Reinhard
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
#include "editorpage.h"
#include "core.h"
#include "ui_GCodeEditor.h"
#include "gcodeeditor.h"
#include "gcodehighlighter.h"
#include <QDebug>
#include <QFileDialog>


EditorPage::EditorPage(QWidget *parent)
 : QWidget(parent)
 , ui(new Ui::GCodeEditorPage)
 , ed(new GCodeEditor)
 , gh(new GCodeHighlighter(ed->document())) {
  ui->setupUi(this);
  connect(ui->pbOpen, &QPushButton::clicked, this, &EditorPage::openFile);
  connect(ui->pbSave, &QPushButton::clicked, this, &EditorPage::saveFile);
  connect(ed->document(), &QTextDocument::modificationChanged, this, &EditorPage::dirtyChanged);
  ui->gridLayout->replaceWidget(ui->widget, ed);
  }


QString EditorPage::chooseGCodeFile(QWidget* parent) {
  QString     selectedFilter, fileName;
  QFileDialog dialog(parent
                   , tr("QFileDialog::getOpenFileName()")
                   , "/media/Scratch"
                   , tr("Fanuc Files (*.dnc);;Heidenhain Files (*.h);;Sinumeric Files (*.arc)"));

  dialog.setSupportedSchemes(QStringList(QStringLiteral("file")));
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setOption(QFileDialog::DontUseNativeDialog);

  if (dialog.exec() != QDialog::Accepted) return QString();
  selectedFilter = dialog.selectedNameFilter();
  fileName       = dialog.selectedUrls().value(0).toLocalFile();

  qDebug() << "choosen GCode file is:" << fileName;

  return fileName;
  }


void EditorPage::dirtyChanged(bool dirty) {
  ui->pbSave->setEnabled(dirty);
  if (dirty) ui->fileName->setText(this->fileName + " [*]");
  else       ui->fileName->setText(this->fileName);
  }


void EditorPage::loadFile(const QString &fileName) {
  if (ed->loadFile(fileName)) {
     ui->fileName->setText(fileName);
     }
  }


void EditorPage::openFile() {
  QString fileName = chooseGCodeFile(this);

  if (fileName.isEmpty()) return;
  loadFile(fileName);
  }


void EditorPage::saveFile() {
  qDebug() << "have to save file:" << ui->fileName->text();
  if (Core().move2Backup(fileName)) {
     QString content = ed->document()->toPlainText();
     QFile   of(fileName);

     if (of.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&of);

//        content.replace(',', '.');
        out << content;
        of.flush();
        of.close();
        QTextBlock  b = ed->textCursor().block();

        ed->setPlainText(content);
        ed->setTextCursor(QTextCursor(b));
//        fileUpdated(fileName);
        }
     else Core().riseError(tr("Failed to write file %1").arg(fileName));
     }
  else Core().riseError(tr("Failed to create backup of file %1").arg(fileName));
  }


void EditorPage::showEvent(QShowEvent *event) {
  ed->setFocus();
  }
