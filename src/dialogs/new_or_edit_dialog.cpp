/*
 * Copyright 2023 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "new_or_edit_dialog.h"

#include "src/main/settings.h"

#include <QPushButton>
#include <QMessageBox>



NewOrEditDialog::NewOrEditDialog(QWidget* parent, Database* db, DialogPurpose purpose):
		QDialog(parent),
		parent(parent),
		db(db),
		purpose(purpose)
{}



void NewOrEditDialog::changeStringsForEdit(QPushButton* okButton)
{
	if (purpose != editItem) return;
	setWindowTitle(getEditWindowTitle());
	okButton->setText(tr("Save changes"));
}



void NewOrEditDialog::handle_ok(QLineEdit* nameLineEdit, QString initName, QString emptyNameWindowTitle, QString emptyNameMessage, const Column* nameColumn)
{
	aboutToClose();
	
	QString itemName = nameLineEdit->text();
	if (itemName.isEmpty() && !Settings::allowEmptyNames.get()) {
		QMessageBox::information(this, emptyNameWindowTitle, emptyNameMessage, QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
	
	if (Settings::warnAboutDuplicateNames.get() && (purpose != editItem || itemName != initName)) {
		// only check for duplicates if the name has changed
		bool proceed = checkNameForDuplicatesAndWarn(itemName, nameColumn);
		if (!proceed) return;	// abort saving
	}
	
	accept();
}

void NewOrEditDialog::handle_cancel()
{
	aboutToClose();
	
	QMessageBox::StandardButton resultButton = QMessageBox::Yes;
	
	if (changesMade() && Settings::confirmCancel.get()) {
		QString title;
		if (purpose != editItem) {
			title = tr("Discard changes");
		} else {
			title = tr("Discard unsaved data");
		}
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resultButton = QMessageBox::question(this, title, question, options, selected);
	}
	
	if (resultButton == QMessageBox::Yes) {
		QDialog::reject();
	}
}


void NewOrEditDialog::reject()
{
	handle_cancel();
}



bool NewOrEditDialog::checkNameForDuplicatesAndWarn(QString name, const Column* nameColumn)
{
	if (!nameColumn->anyCellMatches(name)) {
		return true;	// Item name is not a duplicate, proceed with save
	}
	
	QString windowTitle = tr("Duplicate item name");
	QString message = tr("There is already an item with this name.\nDo you want to save it anyway?");
	auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
	auto selected = QMessageBox::Cancel;
	QMessageBox::StandardButton resultButton = QMessageBox::No;
	resultButton = QMessageBox::warning(this, windowTitle, message, options, selected);
	return resultButton == QMessageBox::Yes;
}





bool displayDeleteWarning(QWidget* parent, QString windowTitle, const QList<WhatIfDeleteResult>& whatIfResults)
{
	QString question;
	if (whatIfResults.isEmpty()) {
		question = NewOrEditDialog::tr("Are you sure you want to delete this item?");
	} else {
		QString whatIfResultString = getTranslatedWhatIfDeleteResultDescription(whatIfResults);
		question = whatIfResultString + "\n" + NewOrEditDialog::tr("Are you sure?");
	}
	
	QMessageBox::StandardButton resultButton = QMessageBox::Yes;
	auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
	auto selected = QMessageBox::Cancel;
	resultButton = QMessageBox::question(parent, windowTitle, question, options, selected);
	return resultButton == QMessageBox::Yes;
}



void populateItemCombo(NormalTable* table, const Column* displayAndSortColumn, bool sortAsString, QComboBox* combo, QList<ValidItemID>& idList)
{
	combo->clear();
	idList.clear();
	combo->addItem(table->getNoneString());
	
	// Get pairs of ID and display/sort field
	QList<QPair<ValidItemID, QVariant>> selectableRanges = table->pairIDWith(displayAndSortColumn);
	
	// Sort entries according to sort field
	auto comparator = [sortAsString] (const QPair<ValidItemID, QVariant>& p1, const QPair<ValidItemID, QVariant>& p2) {
		if (sortAsString) return QString::localeAwareCompare(p1.second.toString(), p2.second.toString()) < 0;
		return QVariant::compare(p1.second, p2.second) == QPartialOrdering::Less;
	};
	std::sort(selectableRanges.begin(), selectableRanges.end(), comparator);
	
	// Save IDs and populate combo box
	for (const QPair<ValidItemID, QVariant>& pair : selectableRanges) {
		idList.append(pair.first);
		combo->addItem(pair.second.toString());
	}
}
