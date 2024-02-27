/*
 * Copyright 2023-2024 Simon Vetter
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

#include "src/settings/settings.h"

#include <QPushButton>
#include <QMessageBox>



/**
 * Creates a new base item dialog.
 * 
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @param purpose		The purpose of the dialog.
 * @param windowTitle	The title of the dialog window.
 */
ItemDialog::ItemDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, const QString& windowTitle):
	QDialog(&parent),
	parent(parent),
	mainWindow(mainWindow),
	db(db),
	purpose(purpose),
	multiEditCheckboxes(QMap<QCheckBox*, QSet<QWidget*>>()),
	tristateCheckboxes(QSet<QCheckBox*>()),
	savedWidgetEnabledStates(QMap<QCheckBox*, QMap<QWidget*, bool>>())
{
	setWindowTitle(windowTitle);
}



/**
 * Sets the pointers to the UI elements that are relevant for editing multiple items at once.
 * 
 * @param saveButton			The dialog's save button.
 * @param multiEditCheckboxes	The checkboxes that control whether to edit a value for all selected items.
 * @param tristateCheckboxes	The checkboxes for bool values in items, which are turned into tristate checkboxes when editing multiple items.
 */
void ItemDialog::setUIPointers(QPushButton* saveButton, const QMap<QCheckBox*, QSet<QWidget*>>& multiEditCheckboxes, const QSet<QCheckBox*>& tristateCheckboxes)
{
	this->saveButton = saveButton;
	this->multiEditCheckboxes = multiEditCheckboxes;
	this->tristateCheckboxes = tristateCheckboxes;
}



/**
 * Makes changes to the UI which are specific to the purpose of the dialog.
 */
void ItemDialog::changeUIForPurpose()
{
	if (purpose == multiEdit) {
		for (const auto& [checkbox, widgets] : multiEditCheckboxes.asKeyValueRange()) {
			checkbox->setToolTip(tr("Set for all selected items"));
			for (QWidget* widget : qAsConst(widgets)) {
				savedWidgetEnabledStates[checkbox][widget] = widget->isEnabled();
				widget->setEnabled(false);
			}
			connect(checkbox, &QPushButton::clicked, this, &ItemDialog::handle_multiEditCheckboxClicked);
		}
		for (QCheckBox* const checkbox : qAsConst(tristateCheckboxes)) {
			checkbox->setTristate(true);
			checkbox->setCheckState(Qt::CheckState::PartiallyChecked);
		}
		saveButton->setText(tr("Save changes for all"));
	}
	else {
		const QList<QCheckBox*> checkboxes = multiEditCheckboxes.keys();
		for (QCheckBox* checkbox : checkboxes) {
			assert(checkbox);
			checkbox->setVisible(false);
		}
	}
	
	if (purpose == editItem) {
		saveButton->setText(tr("Save changes"));
	}
}

/**
 * Generic event handler for clicks on multi-edit checkboxes.
 * 
 * Enables or disables the widgets that are controlled by the checkbox, depending on its state.
 */
void ItemDialog::handle_multiEditCheckboxClicked()
{
	QCheckBox* const checkbox = (QCheckBox*) QObject::sender();
	const QSet<QWidget*>& widgets = multiEditCheckboxes.value(checkbox);
	for (QWidget* const widget : widgets) {
		if (checkbox->isChecked()) {
			// Restore state
			QMap<QWidget*, bool>& enabledStateMap = savedWidgetEnabledStates[checkbox];
			const bool previousState = enabledStateMap.value(widget);
			enabledStateMap.remove(widget);
			widget->setEnabled(previousState);
		} else {
			// Save state and disable
			savedWidgetEnabledStates[checkbox][widget] = widget->isEnabled();
			widget->setEnabled(false);
		}
	}
}



/**
 * Prepares the dialog for closing, performs checks for and warns about empty and duplicate names
 * if settings require it, then accepts the dialog.
 */
void ItemDialog::handle_ok(QLineEdit* nameLineEdit, QString initName, QString emptyNameWindowTitle, QString emptyNameMessage, const ValueColumn& nameColumn)
{
	aboutToClose();
	
	QString itemName = nameLineEdit->text();
	if (itemName.isEmpty()) {
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
bool ItemDialog::checkNameForDuplicatesAndWarn(QString name, const ValueColumn& nameColumn)
{
	if (!nameColumn.anyCellMatches(name)) {
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
bool displayDeleteWarning(QWidget& parent, const QString& windowTitle, const QList<WhatIfDeleteResult>& whatIfResults)
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
	resultButton = QMessageBox::question(&parent, windowTitle, question, options, selected);
	return resultButton == QMessageBox::Yes;
}



/**
 * Repopulates the given combo box with the given table's entries, filtered and sorted according to
 * the given parameters, and writes the IDs of the entries to the referenced list.
 *
 * @param combo						The combo box to populate.
 * @param displayAndSortColumn		The column of the table to use for displaying and sorting the entries.
 * @param idList					The list in which to store the IDs of the entries.
 * @param overrideFirstLine			If not empty, this string will be used as the first line of the combo box instead of the table's none string.
 * @param distinctionKeyColumn		If not null, this column will be used to point to a cell in distinctionContentColumn.
 * @param distinctionContentColumn	If not null, this column will be used to add a distinguishing name in brackets to entries with duplicate names.
 * @param filterColumn				If not null, only entries whose foreign key ID in this column matches the given ID will be added to the combo box.
 * @param filterID					The ID to use for filtering entries, or an invalid ID to filter for entries with no reference.
 * @param prefixColumn				If not null, this column will be used to add a prefix to the display names of the entries. Must be in the same table as displayAndSortColumn.
 * @param prefixValueToString		A function to convert the value of prefixColumn to a string. Must be provided iff prefixColumn is not null.
 */
void populateItemCombo(QComboBox& combo, const ValueColumn& displayAndSortColumn, QList<ValidItemID>& idList, const QString& overrideFirstLine, const ForeignKeyColumn* distinctionKeyColumn, const ValueColumn* distinctionContentColumn, const ForeignKeyColumn* filterColumn, ItemID filterID, const ValueColumn* prefixColumn, std::function<QString (const QVariant&)> prefixValueToString)
{
	assert(!(!filterColumn && filterID.isValid()));
	assert((bool) prefixColumn == (bool) prefixValueToString);
	
	const NormalTable& table = (NormalTable&) displayAndSortColumn.table;
	
	combo.clear();
	idList.clear();
	QString noneString = table.getNoneString();
	if (!overrideFirstLine.isEmpty()) noneString = overrideFirstLine;
	combo.addItem(noneString);
	
	// Get pairs of ID and display/sort field
	QList<QPair<ValidItemID, QVariant>> selectableItems = table.pairIDWith(displayAndSortColumn);
	if (selectableItems.isEmpty()) return;
	
	if (filterColumn) {
		// Filter entries: if an item's foreign key ID doesn't match the given one, discard it
		assert(&filterColumn->table == &table && filterColumn->type == ID);
		for (int i = selectableItems.size() - 1; i >= 0; i--) {
			const ValidItemID& itemID = selectableItems.at(i).first;
			ItemID itemFilterColumnID = filterColumn->getValueFor(itemID);
			if (itemFilterColumnID != filterID) {
				selectableItems.remove(i);
			}
		}
	}
	
	// Add prefix to display names
	if (prefixColumn && prefixValueToString) {
		for (int i = 0; i < selectableItems.size(); i++) {
			const ValidItemID& itemID = selectableItems.at(i).first;
			const QString prefix = prefixValueToString(prefixColumn->getValueFor(itemID));
			if (!prefix.isEmpty()) {
				selectableItems[i].second = prefix + ": " + selectableItems.at(i).second.toString();
			}
		}
	}
	
	// Sort entries according to sort field
	auto comparator = [] (const QPair<ValidItemID, QVariant>& p1, const QPair<ValidItemID, QVariant>& p2) {
		return QString::localeAwareCompare(p1.second.toString(), p2.second.toString()) < 0;
	};
	std::sort(selectableItems.begin(), selectableItems.end(), comparator);
	
	// Add distinction field in brackets in case of duplicates
	if (distinctionKeyColumn || distinctionContentColumn) {
		assert(distinctionKeyColumn && distinctionContentColumn);
		assert(&distinctionKeyColumn->table == &table);
		assert(&distinctionKeyColumn->foreignColumn->table == &distinctionContentColumn->table);
		// Find duplicates
		QSet<int> duplicateNameIndices = QSet<int>();
		QString previousName;
		for (int i = 0; i < selectableItems.size(); i++) {
			const QString currentName = selectableItems.at(i).second.toString();
			const bool sameName = currentName == previousName && i != 0;
			if (sameName) {
				duplicateNameIndices.insert(i - 1);
				duplicateNameIndices.insert(i);
			}
			previousName = currentName;
		}
		// Append distinction column content to duplicate names
		for (const int duplicateNameIndex : duplicateNameIndices) {
			const ValidItemID& itemID = selectableItems.at(duplicateNameIndex).first;
			const ItemID& distinctionKey = distinctionKeyColumn->getValueFor(itemID);
			if (distinctionKey.isInvalid()) continue;
			const QString& distinctionContent = distinctionContentColumn->getValueFor(FORCE_VALID(distinctionKey)).toString();
			if (distinctionContent.isEmpty()) continue;
			const QString& currentName = selectableItems.at(duplicateNameIndex).second.toString();
			const QString newName = currentName + " (" + distinctionContent + ")";
			selectableItems[duplicateNameIndex].second = newName;
		}
	}
	
	// Save IDs and populate combo box
	for (const auto& [itemID, name] : selectableItems) {
		idList.append(itemID);
		combo.addItem(name.toString());
	}
}


void populatePeakCombo(Database& db, QComboBox& peakCombo, QList<ValidItemID>& selectablePeakIDs, ItemID regionFilterID)
{
	if (regionFilterID.isValid()) {
		populateItemCombo(peakCombo, db.peaksTable.nameColumn, selectablePeakIDs, QString(), &db.peaksTable.regionIDColumn, &db.regionsTable.nameColumn, &db.peaksTable.regionIDColumn, regionFilterID);
	} else {
		populateItemCombo(peakCombo, db.peaksTable.nameColumn, selectablePeakIDs, QString(), &db.peaksTable.regionIDColumn, &db.regionsTable.nameColumn);
	}
}

void populateTripCombo(Database& db, QComboBox& tripCombo, QList<ValidItemID>& selectableTripIDs)
{
	auto prefixValueToString = [](const QVariant& prefixValue) {
		if (!prefixValue.canConvert<QDate>()) return QString();
		return QString::number(prefixValue.toDate().year());
	};
	
	populateItemCombo(tripCombo, db.tripsTable.nameColumn, selectableTripIDs, QString(), nullptr, nullptr, nullptr, ItemID(), &db.tripsTable.startDateColumn, prefixValueToString);
}

void populateHikerCombo(Database& db, QComboBox& hikerCombo, QList<ValidItemID>& selectableHikerIDs)
{
	populateItemCombo(hikerCombo, db.hikersTable.nameColumn, selectableHikerIDs);
}

void populateRegionCombo(Database& db, QComboBox& regionCombo, QList<ValidItemID>& selectableRegionIDs, bool asFilter)
{
	if (asFilter) {
		populateItemCombo(regionCombo, db.regionsTable.nameColumn, selectableRegionIDs, ItemDialog::tr("All regions (no filter)"), &db.regionsTable.rangeIDColumn, &db.rangesTable.nameColumn);
	} else {
		populateItemCombo(regionCombo, db.regionsTable.nameColumn, selectableRegionIDs, QString(), &db.regionsTable.rangeIDColumn, &db.rangesTable.nameColumn);
	}
}

void populateRangeCombo(Database& db, QComboBox& rangeCombo, QList<ValidItemID>& selectableRangeIDs)
{
	populateItemCombo(rangeCombo, db.rangesTable.nameColumn, selectableRangeIDs);
}

void populateCountryCombo(Database& db, QComboBox& countryCombo, QList<ValidItemID>& selectableCountryIDs)
{
	populateItemCombo(countryCombo, db.countriesTable.nameColumn, selectableCountryIDs);
}
