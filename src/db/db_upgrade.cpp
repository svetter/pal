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
 * @file db_upgrade.cpp
 * 
 * This file defines the DatabaseUpgrader class.
 */

#include "db_upgrade.h"

#include "src/db/db_error.h"
#include "src/settings/settings.h"

#include <QSqlQuery>
#include <QMessageBox>



/**
 * Creates a DatabaseUpgrader.
 * 
 * @param db		The database object.
 * @param parent	The parent window.
 */
DatabaseUpgrader::DatabaseUpgrader(Database* db, QWidget* parent) :
	db(db),
	parent(parent)
{}



/**
 * Determines the version of the open database and upgrades it if necessary.
 * 
 * In the first step, the current version of the database file is determined. If it matches the
 * current version of the application, no upgrade is necessary and the function returns immediately.
 * 
 * Otherwise, the remaining work is split into two phases:
 * The first phase runs before database initialization, the second phase after. This means that the
 * first phase has to adapt the database structure to the current version. Here, the mechanisms
 * provided by the Database class can't be used yet, since the buffers are not yet initialized.
 * Instead, the database structure has to be modified directly using SQL queries.
 * Before the second phase, the caller-provided function executeAfterStructuralUpgrade is called,
 * which is intended for buffer initialization, which includes checking the database structure.
 * Then, in the second phase, changes in the app which do not affect the database structure can be
 * addressed, while also being able to use the usual mechanisms provided by the Database class.
 * 
 * @param executeAfterStructuralUpgrade	A function to execute after the database structure has been upgraded. Intended for buffer initialization, which includes checking the database structure.
 * @return								True if the upgrade was successful or not necessary and the database can be opened, false if opening the database has to be aborted.
 */
bool DatabaseUpgrader::checkDatabaseVersionAndUpgrade(std::function<void ()> executeAfterStructuralUpgrade)
{
	assert(db->databaseLoaded);
	
	const QString currentDbVersion	= determineCurrentDbVersion();
	const QString appVersion		= getAppVersion();
	
	if (isBelowVersion(appVersion, currentDbVersion)) {
		// App is older than database version, show warning
		bool abort = !showOutdatedAppWarningAndBackup(currentDbVersion);
		if (abort) return false;
		executeAfterStructuralUpgrade();
		return true;
	}
	if (!isBelowVersion(currentDbVersion, appVersion)) {
		// No upgrade necessary
		executeAfterStructuralUpgrade();
		return true;
	}
	
	// Determine whether upgrading from the old version will definitely break compatibility
	bool upgradeBreakCompatibility = isBelowVersion(currentDbVersion, "1.2.0");
	
	// Ask user to confirm upgrade and create backup
	bool abort = !promptUserAboutUpgradeAndBackup(currentDbVersion, upgradeBreakCompatibility);
	if (abort) return false;
	
	
	// Variables to carry information over from the first to the second phase
	ItemID v1_2_0_defaultHikerToCarryOver;
	
	
	
	// === BEFORE INITIALIZING BUFFER ===
	// This is where changes to the database structure have to be made.
	// Table buffers can't be used since they are not yet initialized.
	
	// 1.2.0
	// Versions older than this have a different format for the project settings table
	// Check project settings table structure
	if (isBelowVersion(currentDbVersion, "1.2.0")) {
		// Extract only default hiker, ignore everything else (ascent filters)
		v1_2_0_defaultHikerToCarryOver = extractDefaultHikerFromBeforeV1_2_0();
		// Remove old project settings table
		removeSettingsTableFromBeforeV1_2_0();
		// Create new project settings table
		db->settingsTable->createTableInSql(parent);
	}
	
	
	
	// === CALL PROVIDED CODE ===
	// Perform structure checks and initialize buffers
	executeAfterStructuralUpgrade();
	
	
	
	// === AFTER INITIALIZING BUFFERS ===
	// Database format now has to match current model and table buffers can be used.
	
	// 1.2.0
	if (isBelowVersion(currentDbVersion, "1.2.0")) {
		// Save extracted default hiker back to new table, if present
		if (v1_2_0_defaultHikerToCarryOver.isValid()) {
			db->projectSettings->defaultHiker.set(parent, ID_GET(v1_2_0_defaultHikerToCarryOver));
		}
	}
	
	
	// Set new version
	if (isBelowVersion(currentDbVersion, appVersion)) {
		qDebug().noquote().nospace() << "Upgraded database from v" << currentDbVersion << " to v" << appVersion;
		db->projectSettings->databaseVersion.set(parent, appVersion);
		
		showUpgradeSuccessMessage(currentDbVersion, appVersion);
	}
	
	return true;
}



/**
 * Determines the version of the open database.
 * 
 * To detect structural changes in the database, table information is queried directly using SQL.
 * Then, if the structure matches the current model, the version is read from the database settings.
 * 
 * @return	The version of the open database. Approximate if no version string can be read directly from the database.
 */
QString DatabaseUpgrader::determineCurrentDbVersion()
{
	// Versions older than 1.2.0 have a different format for the project settings table
	// Check project settings table structure
	QString queryString = "PRAGMA table_info(ProjectSettings)";
	QSqlQuery query(queryString);
	if (!query.exec()) displayError(parent, query.lastError(), queryString);
	QStringList columnNames;
	while (query.next()) {
		QString columnName = query.value(1).toString();
		columnNames.append(columnName);
	}

	QStringList columnNamesBeforeV1_2_0 = { "projectSettingsID", "defaultHiker", "dateFilter", "peakHeightFilter", "volcanoFilter", "rangeFilter", "hikeKindFilter", "difficultyFilter", "hikerFilter" };
	QStringList columnNamesAfterV1_2_0 = { "projectSettingID", "settingKey", "settingValue" };

	if (columnNames == columnNamesBeforeV1_2_0) {
		return "1.1.1";
	}
	assert(columnNames == columnNamesAfterV1_2_0);

	// Settings table has current format, get version
	queryString = QString(
		"SELECT settingValue FROM ProjectSettings"
		"\nWHERE settingKey='%1'").arg(db->projectSettings->databaseVersion.key);
	query = QSqlQuery();
	if (!query.prepare(queryString) || !query.exec()) {
		displayError(parent, query.lastError(), queryString);
	}
	assert(query.next());
	QString versionString = query.value(0).toString();
	assert(versionString.split(".").size() == 3);
	return versionString;
}



/**
 * Shows a message box informing the user about the upgrade, asking them to confirm it, and creates
 * a backup copy of the project file if so.
 * 
 * @param oldDbVersion						The version of the database file before the upgrade.
 * @param claimOlderVersionsIncompatible	Whether to claim that older versions of the app will be incompatible with the upgraded database.
 * @return									True if the user wants to continue, false otherwise.
 */
bool DatabaseUpgrader::promptUserAboutUpgradeAndBackup(const QString& oldDbVersion, bool claimOlderVersionsIncompatible)
{
	QString filepath = db->getCurrentFilepath();
	QString windowTitle = Database::tr("Database upgrade necessary");
	QString compatibilityStatement = claimOlderVersionsIncompatible
			? Database::tr("After the upgrade, previous versions of PAL will no longer be able to open the file.")
			: Database::tr("After the upgrade, previous versions of PAL might no longer be able to open the file.");
	QString backupNote = Database::tr("Note: A copy of the project file in its current state will be created as a backup.");
	QString message = filepath + "\n\n"
			+ Database::tr("Opening this project requires upgrading its database from version %1 to version %2."
			"\n%3"
			"\n\nDo you want to perform the upgrade now?"
			"\n\n%4")
			.arg(oldDbVersion, getAppVersion(), compatibilityStatement, backupNote);
	auto buttons = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
	auto defaultButton = QMessageBox::Cancel;
	
	auto response = QMessageBox::question(parent, windowTitle, message, buttons, defaultButton);
	
	bool abort = response != QMessageBox::Yes;
	if (abort) return false;
	
	QString confirmationQuestion = Database::tr("Do you want to perform the upgrade anyway?");
	abort = !createFileBackupCopy(confirmationQuestion);
	return !abort;
}

/**
 * Shows a message box informing the user about the outdated app version, asking them to confirm
 * opening the file anyway, and creates a backup copy of the project file if so.
 * 
 * @param dbVersion	The version of the database file before the upgrade.
 * @return			True if the user wants to continue, false otherwise.
 */
bool DatabaseUpgrader::showOutdatedAppWarningAndBackup(const QString& dbVersion)
{
	QString windowTitle = Database::tr("App version outdated");
	QString backupNote = Database::tr("Note: A copy of the project file in its current state will be created as a backup.");
	QString confirmationQuestion = Database::tr("Do you want to open the file anyway?");
	QString message = db->getCurrentFilepath() + "\n\n"
					  + Database::tr("This project file has version %1, while the app has version %2."
									 "\nOpening a file with an older version of PAL can lead to errors, crashes and data corruption."
									 "It is strongly recommended to only use PAL version %1 or newer to open this file."
									 "\n\n%3"
									 "\n\n%4")
							.arg(dbVersion, getAppVersion(), confirmationQuestion, backupNote);
	auto buttons = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
	auto defaultButton = QMessageBox::Cancel;
	
	auto response = QMessageBox::warning(parent, windowTitle, message, buttons, defaultButton);
	bool abort = response != QMessageBox::Yes;
	if (abort) return false;
	
	abort = !createFileBackupCopy(confirmationQuestion);
	return !abort;
}

/**
 * Creates a backup copy of the project file and asks the user whether to continue if the copy
 * fails.
 * 
 * @param confirmationQuestion	Translated string to insert into the message, asking the user whether to continue if the backup failed.
 * @return						True if the backup was created successfully or the user wants to continue anyway, false otherwise.
 */
bool DatabaseUpgrader::createFileBackupCopy(const QString& confirmationQuestion)
{
	// Determine backup filename
	QString filepath = db->getCurrentFilepath();
	QString backupFilepath = filepath + ".bak";
	int backupFileCounter = 1;
	while (QFile(backupFilepath).exists()) {
		backupFilepath = filepath + " (" + QString::number(backupFileCounter++) + ").bak";
	}
	
	// Copy file
	if (!QFile(filepath).copy(backupFilepath)) {
		qDebug() << "File copy failed:" << filepath << "to" << backupFilepath;
		// Ask user whether to continue
		QString windowTitle = Database::tr("Error creating backup");
		QString message = filepath + "\n\n"
				+ Database::tr("An error occurred while trying to create a backup of the project file."
				"\n%1"
				"\n\nNote: You can still create a backup manually before proceeding.")
				.arg(confirmationQuestion);
		auto buttons = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto defaultButton = QMessageBox::Cancel;
		
		auto response = QMessageBox::question(parent, windowTitle, message, buttons, defaultButton);
		
		return response == QMessageBox::Yes;
	}
	return true;
}

/**
 * Shows a message box informing the user about the successful upgrade.
 * 
 * @param previousVersion	The version of the database file before the upgrade.
 * @param newVersion		The version of the database file after the upgrade.
 */
void DatabaseUpgrader::showUpgradeSuccessMessage(const QString& previousVersion, const QString& newVersion)
{
	QString windowTitle = Database::tr("Database upgrade successful");
	QString message = Database::tr("The database was successfully upgraded from version %1 to version %2.")
			.arg(previousVersion, newVersion);
	QMessageBox::information(parent, windowTitle, message);
}



/**
 * Extracts the default hiker from the project settings table of a database file with a version
 * older than 1.2.0.
 * 
 * @return	The ID of the default hiker, or an invalid ID if no default hiker was set.
 */
ItemID DatabaseUpgrader::extractDefaultHikerFromBeforeV1_2_0()
{
	QString queryString = QString(
		"SELECT defaultHiker FROM ProjectSettings"
		"\nWHERE projectSettingsID=1");
	QSqlQuery query = QSqlQuery();
	if (!query.prepare(queryString) || !query.exec()) {
		displayError(parent, query.lastError(), queryString);
	}
	assert(query.next());
	QVariant value = query.value(0);
	return ItemID(value);
}

/**
 * Removes the project settings table from a database file with a version older than 1.2.0.
 */
void DatabaseUpgrader::removeSettingsTableFromBeforeV1_2_0()
{
	QString queryString = QString("DROP TABLE ProjectSettings");
	QSqlQuery query = QSqlQuery();
	if (!query.prepare(queryString) || !query.exec()) {
		displayError(parent, query.lastError(), queryString);
	}
}
