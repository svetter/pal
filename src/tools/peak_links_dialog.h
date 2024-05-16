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
 * @file peak_links_dialog.h
 * 
 * This file declares the PeakLinksDialog class.
 */

#ifndef PEAK_LINKS_DIALOG_H
#define PEAK_LINKS_DIALOG_H

#include "src/db/database.h"
#include "ui_peak_links_dialog.h"
#include "peak_link_finder_thread.h"

#include <QDialog>
#include <QThread>



/**
 * Control class for the peak links mass auto-detection dialog.
 */
class PeakLinksDialog : public QDialog, public Ui_PeakLinksDialog
{
	Q_OBJECT
	
	/** The project database. */
	Database& db;
	
	/** Indicates whether the worker thread is running. */
	bool running;
	/** The worker thread. */
	PeakLinkFinderThread* workerThread;
	
public:
	PeakLinksDialog(QWidget& parent, Database& db);
	
private slots:
	void handle_start();
	void handle_finished();
	void handle_abort();
	void handle_close();
	
	void handle_callback_workloadSize(int workloadSize);
	void handle_callback_progressUpdate(int processed);
	void handle_callback_updateLinksAt(BufferRowIndex bufferRowIndex, QString mapsLink, QString earthLink, QString wikiLink);
	
private:
	void updateEnableUI();
	
	void reject() override;
};



#endif // PEAK_LINKS_DIALOG_H
