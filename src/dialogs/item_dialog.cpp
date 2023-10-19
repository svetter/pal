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

#include "item_dialog.h"

#include "src/main/settings.h"

#include <QPushButton>
#include <QMessageBox>



ItemDialog::ItemDialog(QWidget* parent, Database* db, DialogPurpose purpose):
		QDialog(parent),
		parent(parent),
		db(db),
		purpose(purpose)
{}



void ItemDialog::changeStringsForEdit(QPushButton* okButton)
{
	if (purpose != editItem) return;
	setWindowTitle(getEditWindowTitle());
	okButton->setText(tr("Save changes"));
}



void ItemDialog::handle_ok(QLineEdit* nameLineEdit, QString initName, QString emptyNameWindowTitle, QString emptyNameMessage, const Column* nameColumn)
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

void ItemDialog::handle_cancel()
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


void ItemDialog::reject()
{
	handle_cancel();
}



bool ItemDialog::checkNameForDuplicatesAndWarn(QString name, const Column* nameColumn)
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
		question = ItemDialog::tr("Are you sure you want to delete this item?");
	} else {
		QString whatIfResultString = getTranslatedWhatIfDeleteResultDescription(whatIfResults);
		question = whatIfResultString + "\n" + ItemDialog::tr("Are you sure?");
	}
	
	QMessageBox::StandardButton resultButton = QMessageBox::Yes;
	auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
	auto selected = QMessageBox::Cancel;
	resultButton = QMessageBox::question(parent, windowTitle, question, options, selected);
	return resultButton == QMessageBox::Yes;
}



void populateItemCombo(NormalTable* table, const Column* displayAndSortColumn, bool sortAsString, QComboBox* combo, QList<ValidItemID>& idList, QString overrideFirstLine, const Column* filterColumn, ItemID filterID)
{
	combo->clear();
	idList.clear();
	QString noneString = table->getNoneString();
	if (!overrideFirstLine.isEmpty()) noneString = overrideFirstLine;
	combo->addItem(noneString);
	
	// Get pairs of ID and display/sort field
	QList<QPair<ValidItemID, QVariant>> selectableItems = table->pairIDWith(displayAndSortColumn);
	
	if (filterColumn && filterID.isValid()) {
		// Filter entries: if an item's foreign key ID doesn't match the given one, discard it
		assert(filterColumn->table == table && filterColumn->type == ID);
		for (int i = selectableItems.size() - 1; i >= 0; i--) {
			const ValidItemID& itemID = selectableItems.at(i).first;
			ItemID itemFilterColumnID = filterColumn->getValueFor(itemID);
			if (itemFilterColumnID != filterID) {
				selectableItems.remove(i);
			}
		}
	}
	
	// Sort entries according to sort field
	auto comparator = [sortAsString] (const QPair<ValidItemID, QVariant>& p1, const QPair<ValidItemID, QVariant>& p2) {
		if (sortAsString) return QString::localeAwareCompare(p1.second.toString(), p2.second.toString()) < 0;
		return QVariant::compare(p1.second, p2.second) == QPartialOrdering::Less;
	};
	std::sort(selectableItems.begin(), selectableItems.end(), comparator);
	
	// Save IDs and populate combo box
	for (const QPair<ValidItemID, QVariant>& pair : selectableItems) {
		idList.append(pair.first);
		combo->addItem(pair.second.toString());
	}
}
