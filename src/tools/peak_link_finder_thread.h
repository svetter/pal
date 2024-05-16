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
 * @file peak_link_finder_thread.h
 * 
 * This file declares the PeakLinkFinderThread class.
 */

#ifndef PEAK_LINK_FINDER_THREAD_H
#define PEAK_LINK_FINDER_THREAD_H

#include "src/db/database.h"

#include <QThread>
#include <QDialog>



/**
 * A thread that relocates photos in the project database.
 */
class PeakLinkFinderThread : public QThread
{
	Q_OBJECT
	
	/** The parent window of this thread. */
	QDialog* parent;
	/** The project database. */
	Database& db;
	/** The Google Search API key to use. */
	const QString apiKey;
	
	/** Indicates whether the thread should update the Google Maps links. */
	const bool maps;
	/** Indicates whether the thread should update the Google Earth links. */
	const bool earth;
	/** Indicates whether the thread should update the Wikipedia links. */
	const bool wiki;
	/** Indicates whether the thread should replace existing links. */
	const bool replaceExisting;
	
	/** The number of peaks to process. */
	int workloadSize;
	/** Indicates whether an abort was requested. */
	bool abortWasCalled;
	
public:
	PeakLinkFinderThread(QDialog* parent, Database& db, bool maps, bool earth, bool wiki, bool replaceExisting);
	
	void run() override;
	void abort();
	
private:
	QString searchForLink(const Peak& peak, const QString website);
	
signals:
	/**
	 * Emitted when the thread has started and determined the number of peaks to process.
	 * 
	 * @param workloadSize	The number of peaks to process.
	 */
	void callback_reportWorkloadSize(int workloadSize);
	/**
	 * Emitted whenever the thread has processed one peak.
	 * 
	 * @param processed	The number of peaks processed so far.
	 */
	void callback_reportProgress(int processed);
	
	/**
	 * Emitted when the thread has found a photo whose filepath needs to be updated.
	 * 
	 * @param bufferRowIndex	The row index of the peak in the buffer.
	 * @param mapsLink			The new Google Maps link for the peak. An empty string if not to be updated.
	 * @param earthLink			The new Google Earth link for the peak. An empty string if not to be updated.
	 * @param wikiLink			The new Wikipedia link for the peak. An empty string if not to be updated.
	 */
	void callback_updateLinksAt(BufferRowIndex bufferRowIndex, QString mapsLink, QString earthLink, QString wikiLink);
};



#endif // PEAK_LINK_FINDER_THREAD_H
