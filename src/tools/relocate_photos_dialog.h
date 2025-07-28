/*
 * Copyright 2023-2025 Simon Vetter
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
 * @file relocate_photos_dialog.h
 * 
 * This file declares the RelocatePhotosDialog class.
 */

#ifndef RELOCATE_PHOTOS_DIALOG_H
#define RELOCATE_PHOTOS_DIALOG_H

#include "src/db/database.h"
#include "src/tools/photo_relocation_thread.h"
#include "ui_relocate_photos_dialog.h"

#include <QDialog>
#include <QThread>



/**
 * Control class for the photo relocation dialog.
 */
class RelocatePhotosDialog : public QDialog, public Ui_RelocatePhotosDialog
{
	Q_OBJECT
	
	/** The project database. */
	Database& db;
	
	/** Indicates whether the worker thread is running. */
	bool running;
	/** The worker thread. */
	PhotoRelocationThread* workerThread;
	
public:
	RelocatePhotosDialog(QWidget& parent, Database& db);
	
private slots:
	void handle_browseOldPath();
	void handle_browseNewPath();
	
	void handle_start();
	void handle_finished();
	void handle_abort();
	void handle_close();
	
	void handle_callback_workloadSize(int workloadSize);
	void handle_callback_progressUpdate(int processed, int updated);
	void handle_callback_updateFilepath(BufferRowIndex bufferRowIndex, QString newFilepath);
	
private:
	void updateEnableUI();
	
	void reject() override;
};



#endif // RELOCATE_PHOTOS_DIALOG_H
