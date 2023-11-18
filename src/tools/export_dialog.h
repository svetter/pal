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
 * @file export_dialog.h
 * 
 * This file declares the DataExportDialog class.
 */

#ifndef EXPORT_DIALOG_H
#define EXPORT_DIALOG_H

#include "ui_export_dialog.h"
#include "src/db/database.h"
#include "src/tools/export_thread.h"

#include <QDialog>
#include <QButtonGroup>



class DataExportDialog : public QDialog, public Ui_DataExportDialog
{
	Q_OBJECT
	
	/** The project database. */
	Database* db;
	
	const ItemTypesHandler* typesHandler;
	
	/** Indicates whether the worker thread is running. */
	bool running;
	/** The worker thread. */
	DataExportThread* workerThread;
	
	QMap<ExportFormat, QString> extensions;
	QList<QString> csvSeparators;
	
public:
	DataExportDialog(QWidget* parent, Database* db, const ItemTypesHandler* typesHandler);
	
private slots:
	void handle_filepathChanged();
	void handle_browseFilepath();
	void handle_fileFormatChanged();
	
	void handle_start();
	void handle_finished();
	void handle_abort();
	void handle_close();
	
	void handle_callback_workloadSize(int workloadSize);
	void handle_callback_progressUpdate(int processed);
	void handle_callback_progressTextUpdate(const QString& progressString);
	
private:
	void updateEnableUI();
	
	ExportMode getCurrentlySelectedExportMode() const;
	ExportFormat getCurrentlySelectedFileFormat() const;
	QString getCurrentlySelectedFileFormatExtension() const;
	const QString& getCurrentlySelectedCsvSeparator() const;
	
	QString enforceExtension(const QString& filepath) const;
	void setFileFormatFromExtension(const QString& filepath) const;
	
	void reject() override;
};



#endif // EXPORT_DIALOG_H
