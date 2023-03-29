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

#ifndef PHOTO_RELOCATION_THREAD_H
#define PHOTO_RELOCATION_THREAD_H

#include "src/db/database.h"

#include <QThread>
#include <QDialog>



class PhotoRelocationThread : public QThread
{
	Q_OBJECT
	
	QDialog* parent;
	
	Database* db;
	QString oldPrefix;
	QString newPrefix;
	
	int workloadSize;
	bool abortWasCalled;
	
public:
	PhotoRelocationThread(QDialog* parent, Database* db, QString oldPrefix, QString newPrefix);
	
	void run() override;
	void abort();
	
signals:
	int reportWorkloadSize(int workloadSize);
	void reportProgress(int processed, int updated);
	
	void callback_updateFilepathAt(int bufferRowIndex, QString newFilepath);
};



#endif // PHOTO_RELOCATION_THREAD_H
