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
 * @file photo_relocation_thread.cpp
 * 
 * This file defines the PhotoRelocationThread class.
 */

#include "photo_relocation_thread.h"



/**
 * Creates a new PhotoRelocationThread.
 * 
 * @param parent	The parent window of this thread.
 * @param db		The database to work on.
 * @param oldPrefix	The prefix which to replace in the paths.
 * @param newPrefix	The new prefix to insert in the paths.
 */
PhotoRelocationThread::PhotoRelocationThread(QDialog* parent, Database& db, QString oldPrefix, QString newPrefix) :
	QThread(parent),
	db(db),
	oldPrefix(oldPrefix),
	newPrefix(newPrefix),
	workloadSize(-1),
	abortWasCalled(false)
{}



/**
 * Starts the thread.
 * 
 * Determines workload size and reports it back via the callback signal reportWorkloadSize().
 * Then iterates over all photos and replaces the old prefix with the new one, delegating the
 * actual updates of the database to the callback signal callback_updateFilepathAt().
 * 
 * After each iteration, the progress is reported back via the callback signal reportProgress().
 * 
 * If abort() is called while the thread is running, the thread will stop after completing the
 * current iteration.
 */
void PhotoRelocationThread::run()
{
	workloadSize = db.photosTable.getNumberOfRows();
	emit callback_reportWorkloadSize(workloadSize);
	
	int updatedPaths = 0;
	
	for (BufferRowIndex index = BufferRowIndex(0); index.isValid(workloadSize); index++) {
		if (abortWasCalled) break;
		
		QString currentPath = db.photosTable.filepathColumn.getValueAt(index).toString();
		
		if (currentPath.startsWith(oldPrefix)) {
			QString newPath = currentPath.replace(0, oldPrefix.size(), newPrefix);
			emit callback_updateFilepathAt(index, newPath);
			updatedPaths++;
		}
		
		emit callback_reportProgress(index.get() + 1, updatedPaths);
	}
}



/**
 * Gracefully aborts the thread.
 */
void PhotoRelocationThread::abort() {
	abortWasCalled = true;
}
