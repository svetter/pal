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

#include "photo_relocation_thread.h"



PhotoRelocationThread::PhotoRelocationThread(QDialog* parent, Database* db, QString oldPrefix, QString newPrefix) :
		QThread(parent),
		parent(parent),
		db(db),
		oldPrefix(oldPrefix),
		newPrefix(newPrefix),
		workloadSize(-1),
		abortWasCalled(false)
{}



void PhotoRelocationThread::run()
{
	workloadSize = db->photosTable->getNumberOfRows();
	reportWorkloadSize(workloadSize);
	
	int updatedPaths = 0;
	
	for (int i = 0; i < workloadSize; i++) {
		if (abortWasCalled) break;
		
		QString currentPath = db->photosTable->filepathColumn->getValueAt(i).toString();
		
		if (currentPath.startsWith(oldPrefix)) {
			QString newPath = currentPath.replace(0, oldPrefix.size(), newPrefix);
			callback_updateFilepathAt(i, newPath);
			updatedPaths++;
		}
		
		emit reportProgress(i + 1, updatedPaths);
	}
}



void PhotoRelocationThread::abort() {
	abortWasCalled = true;
}
