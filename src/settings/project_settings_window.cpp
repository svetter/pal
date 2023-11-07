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
 * @file project_settings_window.cpp
 * 
 * Defines the ProjectSettingsWindow class.
 */

#include "project_settings_window.h"

#include "src/dialogs/hiker_dialog.h"
#include "src/dialogs/parse_helper.h"
#include "src/settings/settings.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>



/**
 * Creates a new ProjectSettingsWindow.
 * 
 * @param parent	The parent widow.
 * @param db		The project database.
 * @param firstOpen	Indicates that the window is being opened as part of the project creation process.
 */
ProjectSettingsWindow::ProjectSettingsWindow(QWidget* parent, Database* db, bool firstOpen) :
		QDialog(parent),
		parent(parent),
		db(db),
		firstOpen(firstOpen),
		selectableHikerIDs(QList<ValidItemID>())
{
	setupUi(this);
	setFixedSize(sizeHint());
	
	if (firstOpen) {
		defaultHikerCombo->setVisible(false);
		newHikerButton->setVisible(false);
		bottomButtonBox->removeButton(bottomButtonBox->button(QDialogButtonBox::Apply));
	} else {
		newDefaultHikerLineEdit->setVisible(false);
	}
	
	restoreDialogGeometry(this, parent, &Settings::projectSettingsWindow_geometry);
	
	
	populateItemCombo(db->hikersTable, db->hikersTable->nameColumn, true, defaultHikerCombo, selectableHikerIDs);
	
	
	connect(newHikerButton,										&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_newHiker);
	
	connect(bottomButtonBox->button(QDialogButtonBox::Save),	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_save);
	connect(bottomButtonBox->button(QDialogButtonBox::Cancel),	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_cancel);
	if (!firstOpen)
	connect(bottomButtonBox->button(QDialogButtonBox::Apply),	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_apply);
	
	
	loadSettings();
	
	
	if (firstOpen) {
		QString userName = qgetenv("USER");
		if (userName.isEmpty()) {
			userName = qgetenv("USERNAME");
		}
		if (!userName.isEmpty()) {
			newDefaultHikerLineEdit->setText(userName);
		}
	}
}



/**
 * Loads the project settings from the database into the UI.
 */
void ProjectSettingsWindow::loadSettings()
{
	ItemID hikerID = db->projectSettings->defaultHiker->get();
	if (hikerID.isValid()) {
		defaultHikerCombo->setCurrentIndex(selectableHikerIDs.indexOf(hikerID) + 1);	// 0 is None
	} else {
		defaultHikerCombo->setCurrentIndex(0);
	}
}

/**
 * Saves the project settings from the UI into the database.
 */
void ProjectSettingsWindow::saveSettings()
{
	if (firstOpen && !newDefaultHikerLineEdit->text().isEmpty()) {
		QString newDefaultHikerName = newDefaultHikerLineEdit->text();
		Hiker* newDefaultHiker = new Hiker(ItemID(), newDefaultHikerName);
		BufferRowIndex newHikerIndex = db->hikersTable->addRow(this, newDefaultHiker);
		defaultHikerCombo->setCurrentIndex(newHikerIndex.get() + 1);	// 0 is None
	}
	db->projectSettings->defaultHiker->set(this, parseItemCombo(defaultHikerCombo, selectableHikerIDs).asQVariant());
}



/**
 * Event handler for the new hiker button.
 * 
 * Opens a new hiker dialog, adds the new hiker to the database and selects it in the combo box.
 */
void ProjectSettingsWindow::handle_newHiker()
{
	BufferRowIndex newHikerIndex = openNewHikerDialogAndStore(this, db);
	if (newHikerIndex.isInvalid()) return;
	ValidItemID hikerID = db->hikersTable->getPrimaryKeyAt(newHikerIndex);
	defaultHikerCombo->setCurrentIndex(selectableHikerIDs.indexOf(hikerID) + 1);	// 0 is None
}



/**
 * Event handler for the save button.
 * 
 * Saves the settings and window position and size and closes the window.
 */
void ProjectSettingsWindow::handle_save()
{
	saveSettings();
	saveDialogGeometry(this, parent, &Settings::projectSettingsWindow_geometry);
	accept();
}

/**
 * Event handler for the apply button.
 * 
 * Saves the settings.
 */
void ProjectSettingsWindow::handle_apply()
{
	saveSettings();
}

/**
 * Event handler for the cancel button.
 * 
 * Saves window position and size and closes the window.
 */
void ProjectSettingsWindow::handle_cancel()
{
	saveDialogGeometry(this, parent, &Settings::projectSettingsWindow_geometry);
	QDialog::reject();
}



/**
 * Event handler for the close event.
 *
 * Forwarded to the cancel button event handler.
 */
void ProjectSettingsWindow::reject()
{
	handle_cancel();
}
