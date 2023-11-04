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

/**
 * @file item_dialog.cpp
 * 
 * This file defines the ItemDialog class.
 */

#include "item_dialog.h"

#include "src/main/settings.h"

#include <QPushButton>
#include <QMessageBox>



/**
 * Creates a new base item dialog.
 * 
 * @param parent	The parent window.
 * @param db		The project database.
 * @param purpose	The purpose of the dialog.
 */
ItemDialog::ItemDialog(QWidget* parent, Database* db, DialogPurpose purpose):
		QDialog(parent),
		parent(parent),
		db(db),
		purpose(purpose)
{}



/**
 * Exchanges all relevant strings to prepare the item dialog for editing an existing item.
 * 
 * @param okButton	The ok button.
 */
void ItemDialog::changeStringsForEdit(QPushButton* okButton)
{
	if (purpose != editItem) return;
	setWindowTitle(getEditWindowTitle());
	okButton->setText(tr("Save changes"));
}



/**
 * Prepares the dialog for closing, performs checks for and warns about empty and duplicate names
 * if settings require it, then accepts the dialog.
 */
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

/**
 * Prepares the dialog for closing, checks for and warns about unsaved changes if settings require
 * it, then rejects the dialog if the user chooses to proceed.
 */
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


/**
 * Forwards the cancel event to handle_cancel().
 */
void ItemDialog::reject()
{
	handle_cancel();
}



/**
 * Checks whether the given name already exists in the given column and warns the user if it does.
 * 
 * @param name			The name to check.
 * @param nameColumn	The column to check.
 * @return				True if the name is not a duplicate or the user chooses to proceed anyway, false otherwise.
 */
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





/**
 * Shows a warning message to the user, asking them to confirm the given results of the deletion.
 * 
 * @param parent		The parent window.
 * @param windowTitle	The title of the warning window.
 * @param whatIfResults	The determined results of the deletion.
 * @return				True if the user confirmed the deletion, false otherwise.
 */
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



/**
 * Repopulates the given combo box with the given table's entries, filtered and sorted according to
 * the given parameters, and writes the IDs of the entries to the referenced list.
 * 
 * @param table					The table to get the entries from.
 * @param displayAndSortColumn	The column of the table to use for displaying and sorting the entries.
 * @param sortAsString			Whether to sort the entries as strings. Otherwise, QVariant::compare() is used.
 * @param combo					The combo box to populate.
 * @param idList				The list in which to store the IDs of the entries.
 * @param overrideFirstLine		If not empty, this string will be used as the first line of the combo box instead of the table's none string.
 * @param filterColumn			If not null, only entries whose foreign key ID in this column matches the given ID will be added to the combo box.
 * @param filterID				The ID to use for filtering entries, or an invalid ID to filter for entries with no reference.
 */
void populateItemCombo(NormalTable* table, const Column* displayAndSortColumn, bool sortAsString, QComboBox* combo, QList<ValidItemID>& idList, QString overrideFirstLine, const Column* filterColumn, ItemID filterID)
{
	assert(!(!filterColumn && filterID.isValid()));
	
	combo->clear();
	idList.clear();
	QString noneString = table->getNoneString();
	if (!overrideFirstLine.isEmpty()) noneString = overrideFirstLine;
	combo->addItem(noneString);
	
	// Get pairs of ID and display/sort field
	QList<QPair<ValidItemID, QVariant>> selectableItems = table->pairIDWith(displayAndSortColumn);
	
	if (filterColumn) {
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
