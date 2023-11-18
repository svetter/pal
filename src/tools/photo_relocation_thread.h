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
 * @file photo_relocation_thread.h
 * 
 * This file declares the PhotoRelocationThread class.
 */

#ifndef PHOTO_RELOCATION_THREAD_H
#define PHOTO_RELOCATION_THREAD_H

#include "src/db/database.h"

#include <QThread>
#include <QDialog>



/**
 * A thread that relocates photos in the project database.
 */
class PhotoRelocationThread : public QThread
{
	Q_OBJECT
	
	/** The project database. */
	Database* db;
	/** The old prefix to replace in the photo filepaths. */
	QString oldPrefix;
	/** The new prefix for the photo filepaths. */
	QString newPrefix;
	
	/** The number of photos to relocate. */
	int workloadSize;
	/** Indicates whether an abort was requested. */
	bool abortWasCalled;
	
public:
	PhotoRelocationThread(QDialog* parent, Database* db, QString oldPrefix, QString newPrefix);
	
	void run() override;
	void abort();
	
signals:
	/**
	 * Emitted when the thread has started and determined the number of photos to relocate.
	 * 
	 * @param workloadSize	The number of photos to relocate.
	 */
	void callback_reportWorkloadSize(int workloadSize);
	/**
	 * Emitted whenever the thread has processed one photo.
	 * 
	 * @param processed	The number of photos processed so far.
	 * @param updated	The number of photos whose filepath was updated so far.
	 */
	void callback_reportProgress(int processed, int updated);
	
	/**
	 * Emitted when the thread has found a photo whose filepath needs to be updated.
	 * 
	 * @param bufferRowIndex	The row index of the photo in the buffer.
	 * @param newFilepath		The new filepath for the photo.
	 */
	void callback_updateFilepathAt(BufferRowIndex bufferRowIndex, QString newFilepath);
};



#endif // PHOTO_RELOCATION_THREAD_H
