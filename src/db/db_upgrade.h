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
 * @file db_upgrade.h
 * 
 * This file declares the DatabaseUpgrader class.
 */

#ifndef DB_UPGRADE_H
#define DB_UPGRADE_H

#include "src/db/database.h"

#include <QWidget>



/**
 * A class encapsulating all code for upgrading database files created by older versions of the app.
 */
class DatabaseUpgrader
{
	/** The database object. */
	Database* const db;
	/** The parent window. */
	QWidget* const parent;
	
public:
	DatabaseUpgrader(Database* db, QWidget* parent);
	
	bool checkDatabaseVersionAndUpgrade(std::function<void ()> executeAfterStructuralUpgrade);
	
private:
	QString determineCurrentDbVersion();
	bool promptUserAboutUpgradeAndBackup(const QString& oldDbVersion, bool claimOlderVersionsIncompatible);
	bool showOutdatedAppWarningAndBackup(const QString& dbVersion);
	bool createFileBackupCopy(const QString& confirmationQuestion, const QString& currentDbVersion);
	void showUpgradeSuccessMessage(const QString& previousVersion, const QString& newVersion);
	
	// Version-specific functions
	ItemID extractDefaultHikerFromBeforeV1_2_0();
	void removeSettingsTableFromBeforeV1_2_0();
};



#endif // DB_UPGRADE_H
