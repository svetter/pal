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
 * @file range_dialog.cpp
 * 
 * This file defines the RangeDialog class.
 */

#include "range_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/settings/settings.h"
#include "src/data/enum_names.h"

#include <QMessageBox>



/**
 * Creates a new range dialog.
 * 
 * Sets up the UI, restores geometry, populates combo boxes, connects interactive UI elements, and
 * performs purpose-specific preparations.
 * 
 * @param parent	The parent window.
 * @param db		The project database.
 * @param purpose	The purpose of the dialog.
 * @param init		The range data to initialize the dialog with and store as initial data. RangeDialog takes ownership of this pointer.
 */
RangeDialog::RangeDialog(QWidget* parent, Database* db, DialogPurpose purpose, Range* init) :
	ItemDialog(parent, db, purpose),
	init(init)
{
	setupUi(this);
	setWindowIcon(QIcon(":/icons/ico/range_multisize_square.ico"));
	
	restoreDialogGeometry(this, parent, &Settings::rangeDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	
	populateComboBoxes();
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&RangeDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&RangeDialog::handle_cancel);
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
		changeStringsForEdit(okButton);
		insertInitData();
		break;
	default:
		assert(false);
	}
}

/**
 * Destroys the range dialog.
 */
RangeDialog::~RangeDialog()
{
	delete init;
}



/**
 * Returns the window title to use when the dialog is used to edit an item.
 *
 * @return	The window title for editing an item
 */
QString RangeDialog::getEditWindowTitle()
{
	return tr("Edit mountain range");
}



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
Range* RangeDialog::extractData()
{
	QString	name		= parseLineEdit		(nameLineEdit);
	int		continent	= parseEnumCombo	(continentCombo, true);
	
	Range* range = new Range(ItemID(), name, continent);
	return range;
}


/**
 * Checks whether changes have been made to the range, compared to the initial range object, if
 * set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool RangeDialog::changesMade()
{
	Range* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
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
	const ValueColumn* nameColumn = db->rangesTable->nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void RangeDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::rangeDialog_geometry);
}





static BufferRowIndex openRangeDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Range* originalRange);

/**
 * Opens a new range dialog and saves the new range to the database.
 *
 * @param parent	The parent window.
 * @param db		The project database.
 * @return			The index of the new range in the database's range table buffer.
 */
BufferRowIndex openNewRangeDialogAndStore(QWidget* parent, Database* db)
{
	return openRangeDialogAndStore(parent, db, newItem, nullptr);
}

/**
 * Opens an edit range dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the range to edit in the database's range table buffer.
 */
void openEditRangeDialogAndStore(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Range* originalRange = db->getRangeAt(bufferRowIndex);
	openRangeDialogAndStore(parent, db, editItem, originalRange);
}

/**
 * Opens a delete range dialog and deletes the range from the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndices	The indices of the ranges to delete in the database's range table buffer.
 */
void openDeleteRangesDialogAndExecute(QWidget* parent, Database* db, QSet<BufferRowIndex> bufferRowIndices)
{
	if (bufferRowIndices.isEmpty()) return;
	
	QSet<ValidItemID> rangeIDs = QSet<ValidItemID>();
	for (const BufferRowIndex& bufferRowIndex : bufferRowIndices) {
		rangeIDs += VALID_ITEM_ID(db->rangesTable->primaryKeyColumn->getValueAt(bufferRowIndex));
	}
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRows(db->rangesTable, rangeIDs);
	
	if (Settings::confirmDelete.get()) {
		bool plural = rangeIDs.size() > 1;
		QString windowTitle = plural ? RangeDialog::tr("Delete mountain ranges") : RangeDialog::tr("Delete mountain range");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}

	db->removeRows(parent, db->rangesTable, rangeIDs);
}



/**
 * Opens a purpose-generic range dialog and applies the resulting changes to the database.
 *
 * @param parent		The parent window.
 * @param db			The project database.
 * @param purpose		The purpose of the dialog.
 * @param originalRange	The range data to initialize the dialog with and store as initial data. RangeDialog takes ownership of this pointer.
 * @return				The index of the new range in the database's range table buffer. Invalid if the dialog was canceled or the purpose was editItem.
 */
static BufferRowIndex openRangeDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Range* originalRange)
{
	BufferRowIndex newRangeIndex = BufferRowIndex();
	if (purpose == duplicateItem) {
		assert(originalRange);
		originalRange->rangeID = ItemID();
	}
	
	RangeDialog dialog(parent, db, purpose, originalRange);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Range* extractedRange = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newRangeIndex = db->rangesTable->addRow(parent, extractedRange);
			break;
		case editItem:
			db->rangesTable->updateRow(parent, FORCE_VALID(originalRange->rangeID), extractedRange);
			break;
		default:
			assert(false);
		}
		
		delete extractedRange;
	}
	
	return newRangeIndex;
}
