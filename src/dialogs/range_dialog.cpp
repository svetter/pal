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
 * @file range_dialog.cpp
 * 
 * This file defines the RangeDialog class.
 */

#include "range_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/settings/settings.h"
#include "src/data/enum_names.h"

#include <QMessageBox>

using std::unique_ptr, std::make_unique;



/**
 * Creates a new range dialog.
 * 
 * Sets up the UI, restores geometry, populates combo boxes, connects interactive UI elements, and
 * performs purpose-specific preparations.
 * 
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param purpose			The purpose of the dialog.
 * @param init				The range data to initialize the dialog with and store as initial data. RangeDialog takes ownership of this pointer.
 * @param numItemsToEdit	The number of items to edit, if the purpose is multi-edit.
 */
RangeDialog::RangeDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, unique_ptr<const Range> init, int numItemsToEdit) :
	ItemDialog(parent, mainWindow, db, purpose),
	init(std::move(init))
{
	setupUi(this);
	setUIPointers(okButton, {
		{nameCheckbox,		{{ nameLineEdit },		{ &db.rangesTable.nameColumn }}},
		{continentCheckbox,	{{ continentCombo },	{ &db.rangesTable.continentColumn }}}
	});
	
	setWindowIcon(QIcon(":/icons/ico/range_multisize_square.ico"));
	switch (purpose) {
	case newItem:
	case duplicateItem:	setWindowTitle(tr("New mountain range"));								break;
	case editItem:
	case multiEdit:		setWindowTitle(tr("Edit %Ln mountain range(s)", "", numItemsToEdit));	break;
	}
	
	restoreDialogGeometry(*this, mainWindow, Settings::rangeDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	
	populateComboBoxes();
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&RangeDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&RangeDialog::handle_cancel);
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
	case multiEdit:
		insertInitData();
		break;
	default:
		assert(false);
	}
	changeUIForPurpose();
}

/**
 * Destroys the range dialog.
 */
RangeDialog::~RangeDialog()
{}



/**
 * Populates the dialog's combo boxes with data from the database.
 */
void RangeDialog::populateComboBoxes()
{
	continentCombo->insertItems(0, EnumNames::translateList(EnumNames::continentNames));
}



/**
 * Inserts the data from the initial range object into the dialog's UI elements.
 */
void RangeDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
	// Continent
	continentCombo->setCurrentIndex(init->continent);
}


/**
 * Extracts the data from the UI elements and returns it as a range object.
 *
 * @return	The range data as a range object. The caller takes ownership of the object.
 */
unique_ptr<Range> RangeDialog::extractData()
{
	QString	name		= parseLineEdit		(*nameLineEdit);
	int		continent	= parseEnumCombo	(*continentCombo, true);
	
	return make_unique<Range>(ItemID(), name, continent);
}


/**
 * Checks whether changes have been made to the range, compared to the initial range object, if
 * set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool RangeDialog::changesMade()
{
	if (purpose == multiEdit) {
		return anyMultiEditChanges();
	}
	
	return !extractData()->equalTo(*init);
}



/**
 * Event handler for the OK button.
 *
 * Checks whether the name is empty or a duplicate depending on settings, prepares the dialog for
 * closing and then accepts it.
 */
void RangeDialog::handle_ok()
{
	QString emptyNameWindowTitle	= tr("Can't save mountain range");
	QString emptyNameWindowMessage	= tr("The mountain range needs a name.");
	const ValueColumn& nameColumn = db.rangesTable.nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void RangeDialog::aboutToClose()
{
	saveDialogGeometry(*this, mainWindow, Settings::rangeDialog_geometry);
}





/**
 * Opens a new range dialog and saves the new range to the database.
 *
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @param callWhenDone	The function to call after the dialog has closed.
 */
void openNewRangeDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, std::function<void (BufferRowIndex)> callWhenDone)
{
	RangeDialog* dialog = new RangeDialog(parent, mainWindow, db, newItem, nullptr);
	
	auto callWhenClosed = [=, &parent, &db]() {
		BufferRowIndex newRangeIndex = BufferRowIndex();
		
		if (dialog->result() == QDialog::Accepted) {
			unique_ptr<Range> extractedRange = dialog->extractData();
			
			db.beginChangingData();
			newRangeIndex = db.rangesTable.addRow(parent, *extractedRange);
			db.finishChangingData();
		}
		
		delete dialog;
		return callWhenDone(newRangeIndex);
	};
	RangeDialog::connect(dialog, &RangeDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens an edit range dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the range to edit in the database's range table buffer.
 * @param callWhenDone		The function to call after the dialog has closed.
 */
void openEditRangeDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex, std::function<void (bool)> callWhenDone)
{
	unique_ptr<Range> originalRange = db.getRangeAt(bufferRowIndex);
	const ItemID originalRangeID = originalRange->rangeID;
	
	RangeDialog* dialog = new RangeDialog(parent, mainWindow, db, editItem, std::move(originalRange));
	
	auto callWhenClosed = [=, &parent, &db]() {
		bool changesMade = false;
		
		if (dialog->result() == QDialog::Accepted && dialog->changesMade()) {
			unique_ptr<Range> extractedRange = dialog->extractData();
			
			db.beginChangingData();
			db.rangesTable.updateRow(parent, FORCE_VALID(originalRangeID), *extractedRange);
			db.finishChangingData();
			changesMade = true;
		}
		
		delete dialog;
		return callWhenDone(changesMade);
	};
	RangeDialog::connect(dialog, &RangeDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens a multi-edit range dialog and saves the changes to the database.
 * 
 * @param parent				The parent window.
 * @param mainWindow			The application's main window.
 * @param db					The project database.
 * @param bufferRowIndices		The buffer row indices of the ranges to edit.
 * @param initBufferRowIndex	The index of the range whose data to initialize the dialog with.
 * @param callWhenDone			The function to call after the dialog has closed.
 */
void openMultiEditRangesDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices, BufferRowIndex initBufferRowIndex, std::function<void (bool)> callWhenDone)
{
	assert(!bufferRowIndices.isEmpty());
	
	unique_ptr<Range> originalRange = db.getRangeAt(initBufferRowIndex);
	
	RangeDialog* dialog = new RangeDialog(parent, mainWindow, db, multiEdit, std::move(originalRange), bufferRowIndices.size());
	
	auto callWhenClosed = [=, &parent, &db]() {
		bool changesMade = false;
		
		if (dialog->result() == QDialog::Accepted && dialog->changesMade()) {
			unique_ptr<Range> extractedRange = dialog->extractData();
			extractedRange->rangeID = ItemID();
			QSet<const Column*> columnsToSave = dialog->getMultiEditColumns();
			QList<const Column*> columnList = QList<const Column*>(columnsToSave.constBegin(), columnsToSave.constEnd());
			
			db.beginChangingData();
			db.rangesTable.updateRows(parent, bufferRowIndices, columnList, *extractedRange);
			db.finishChangingData();
			changesMade = true;
		}
		
		delete dialog;
		return callWhenDone(changesMade);
	};
	RangeDialog::connect(dialog, &RangeDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens a delete range dialog and deletes the range from the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndices	The indices of the ranges to delete in the database's range table buffer.
 * @return					True if any items were deleted, false otherwise.
 */
bool openDeleteRangesDialogAndExecute(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices)
{
	Q_UNUSED(mainWindow);
	if (bufferRowIndices.isEmpty()) return false;
	
	QSet<ValidItemID> rangeIDs = QSet<ValidItemID>();
	for (const BufferRowIndex& bufferRowIndex : bufferRowIndices) {
		rangeIDs += VALID_ITEM_ID(db.rangesTable.primaryKeyColumn.getValueAt(bufferRowIndex));
	}
	
	QList<WhatIfDeleteResult> whatIfResults = db.whatIf_removeRows(db.rangesTable, rangeIDs);
	
	if (Settings::confirmDelete.get()) {
		const QString windowTitle = RangeDialog::tr("Delete %Ln mountain range(s)", "", rangeIDs.size());
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return false;
	}
	
	db.beginChangingData();
	db.removeRows(parent, db.rangesTable, rangeIDs);
	db.finishChangingData();
	return true;
}
