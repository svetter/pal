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
 * @file export_thread.h
 * 
 * This file declares the DataExportThread class.
 */

#ifndef EXPORT_THREAD_H
#define EXPORT_THREAD_H

#include "src/db/database.h"
#include "src/main/item_types_handler.h"

#include <QDialog>
#include <QThread>
#include <QXmlStreamWriter>



enum ExportMode
{
	Raw,
	Readable
};

enum ExportFormat
{
	CSV,
	FODS
};

struct ExportColumnInfo
{
	int indexInTable;
	QString name;
	DataType type;
	const QStringList* enumNames;
	const QList<QPair<QString, QStringList>>* enumNameLists;
	
	QString styleName = "Default";
};



class DataExportThread : public QThread
{
	Q_OBJECT
	
	const Database* db;
	const ItemTypesHandler* typesHandler;
	const ExportMode mode;
	const bool includeStats;
	const QString filepath;
	const ExportFormat format;
	const QString csvSeparator;
	
	int workloadSize;
	bool abortWasCalled;
	
	QFile* file;
	QTextStream* fileWriter;
	QXmlStreamWriter* xmlWriter;
	
	inline static const QString fods_boldStyleName = "ce1";
	inline static const QString fods_dateStyleName = "ce2";
	inline static const QString fods_timeStyleName = "ce3";
	
public:
	DataExportThread(QDialog* parent, Database* db, const ItemTypesHandler* typesHandler, ExportMode mode, bool includeStats, const QString& filepath, ExportFormat format, QString csvSeparator);
	~DataExportThread();
	
	void run() override;
	void abort();
private:
	void exportRaw();
	void writeEnumTable(const QList<Table*>& baseTables, const QList<QList<ExportColumnInfo>>& allColumnInfos);
	void exportReadable();
	
	// Low-level helpers
	void openFileForWriting(const QString& filepathToOpen);
	
	void beginExport(QList<QList<ExportColumnInfo>>& allColumnInfos, QStringList tableNames);
	void beginTable(const QString& tableName, const QList<ExportColumnInfo>& columnInfos);
	void beginRow();
	void writeCell(const QVariant& value, const ExportColumnInfo& columnInfo, bool firstInRow);
	void endRow();
	void writeEmptyRow(const QList<ExportColumnInfo>& columnInfos);
	void endTable();
	void endExport();
	
	void beginTableCSV(const QString& tableName, const QList<ExportColumnInfo>& columnInfos);
	void beginRowCSV();
	void writeCellCSV(const QVariant& value, const ExportColumnInfo& columnInfo, bool firstInRow);
	void endTableCSV();
	
	void beginExportFODS(QList<QList<ExportColumnInfo>>& allColumnInfos, QStringList tableNames);
	void beginTableFODS(const QString& tableName, const QList<ExportColumnInfo>& columnInfos);
	void beginRowFODS();
	void writeCellFODS(const QVariant& value, const ExportColumnInfo& columnInfo);
	void endRowFODS();
	void endTableFODS();
	void endExportFODS();
	
public:
	static QString insertBeforeExtension(const QString& baseFilepath, const QString& insert);
	
signals:
	void callback_reportWorkloadSize(int workloadSize);
	void callback_setProgressText(QString progressText);
	void callback_reportProgress(int processed);
};



#endif // EXPORT_THREAD_H
