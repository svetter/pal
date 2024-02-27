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
 * @file export_thread.h
 * 
 * This file declares the DataExportThread class.
 */

#ifndef EXPORT_THREAD_H
#define EXPORT_THREAD_H

#include "src/tools/export_decls.h"
#include "export_writer.h"
#include "src/main/item_types_handler.h"

#include <QDialog>
#include <QThread>



/**
 * A thread that exports data to one or multiple files according to the given parameters.
 */
class DataExportThread : public QThread
{
	Q_OBJECT
	
	/** The project item types handler. */
	const ItemTypesHandler* typesHandler;
	/** The export mode. */
	const ExportMode mode;
	/** Whether to include statistical column. */
	const bool includeStats;
	/** The base filepath. */
	const QString filepath;
	
	/** The writer object specific to the export format. */
	ExportWriter* writer;
	
	/** The determined number of cells to export, plus the number of cells to update first. */
	int workloadSize;
	/** Indicates whether an abort was requested. */
	bool abortWasCalled;
	
public:
	DataExportThread(QDialog& parent, const ItemTypesHandler* typesHandler, ExportMode mode, bool includeStats, const QString& filepath, ExportFormat format, QString csvSeparator);
	~DataExportThread();
	
	void run() override;
	void abort();

private:
	void exportOneTable();

	void exportAsShown();

	void exportRaw();
	void writeEnumTable(const QList<Table*>& baseTables, const QList<QList<ExportColumnInfo>>& allColumnInfos);
	
signals:
	void callback_reportWorkloadSize(int workloadSize);
	void callback_setProgressText(QString progressText);
	void callback_reportProgress(int processed);
};



#endif // EXPORT_THREAD_H
