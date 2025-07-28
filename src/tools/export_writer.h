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
 * @file export_decls.h
 * 
 * This file declares the ExportWriter class and its subclasses.
 */

#ifndef EXPORT_WRITER_H
#define EXPORT_WRITER_H

#include "src/tools/export_decls.h"

#include <QDialog>
#include <QThread>
#include <QFile>
#include <QXmlStreamWriter>



/**
 * A superclass for writer classes which abstract the file format specifics from the export worker.
 * 
 * The class is abstract and cannot be instantiated.
 * 
 * @see CsvExportWriter
 * @see FodsExportWriter
 */
class ExportWriter {
protected:
	/** The export mode. */
	const ExportMode mode;
	/** The path and filename (or base filename if multiple files are written) for the export. */
	const QString filepath;
	/** The file to write to at the current point during the export process. */
	QFile* file;
	
	ExportWriter(ExportMode mode, const QString& filepath);
public:
	virtual ~ExportWriter();
	
	/**
	 * Called at the start of the entire export process.
	 * 
	 * @param allColumnInfos	ExportColumnInfo objects for all columns in all tables.
	 * @param tableNames		The translated UI names of all tables.
	 */
	virtual void beginExport(QList<QList<ExportColumnInfo>>& allColumnInfos, QStringList tableNames) = 0;
	/**
	 * Called at the start of a table during the export process.
	 * 
	 * @param tableName		The translated UI name of the table.
	 * @param columnInfos	ExportColumnInfo objects for all columns in the table.
	 */
	virtual void beginTable(const QString& tableName, const QList<ExportColumnInfo>& columnInfos) = 0;
	/**
	 * Called at the start of a row during the export process.
	 */
	virtual void beginRow() = 0;
	/**
	 * Called for each cell during the export process.
	 * 
	 * @param value			The value to write for the cell.
	 * @param columnInfo	The ExportColumnInfo object for the column of the cell.
	 */
	virtual void writeCell(const QVariant& value, const ExportColumnInfo& columnInfo) = 0;
	/**
	 * Called at the end of a row during the export process.
	 */
	virtual void endRow() = 0;
	/**
	 * Called at the end of a table during the export process.
	 */
	virtual void endTable() = 0;
	/**
	 * Called at the end of the entire export process.
	 */
	virtual void endExport() = 0;
	
	virtual void writeEmptyRow(const QList<ExportColumnInfo>& columnInfos);
	
protected:
	void openFileForWriting(const QString& filepathToOpen);
	static QString insertBeforeExtension(const QString& baseFilepath, const QString& insert);
};



/**
 * A subclass of ExportWriter which writes to a CSV file.
 */
class CsvExportWriter : public ExportWriter {
	/** The separator character to use between cells. */
	const QString separator;
	/** A QTextStream for writing to the current file. */
	QTextStream* fileWriter;
	
	/** The number of tables in the entire export. */
	int numTables;
	/** Indicates that one or more cells have already been written for the current row. */
	bool afterFirstItemInRow;
	
public:
	CsvExportWriter(ExportMode mode, const QString& filepath, const QString& separator);
	virtual ~CsvExportWriter();
	
	virtual void beginExport(QList<QList<ExportColumnInfo>>& allColumnInfos, QStringList tableNames);
	virtual void beginTable(const QString& tableName, const QList<ExportColumnInfo>& columnInfos);
	virtual void beginRow();
	virtual void writeCell(const QVariant& value, const ExportColumnInfo& columnInfo);
	virtual void endRow();
	virtual void endTable();
	virtual void endExport();
};



/**
 * A subclass of ExportWriter which writes to a FODS file.
 */
class FodsExportWriter : public ExportWriter {
	/** A QXmlStreamWriter for writing to the current file. */
	QXmlStreamWriter* xmlWriter;
	
	/** The style name to use for the bold style. */
	inline static const QString boldStyleName = "ce1";
	/** The style name to use for the date style. */
	inline static const QString dateStyleName = "ce2";
	/** The style name to use for the time style. */
	inline static const QString timeStyleName = "ce3";
	
public:
	FodsExportWriter(ExportMode mode, const QString& filepath);
	virtual ~FodsExportWriter();
	
	virtual void beginExport(QList<QList<ExportColumnInfo>>& allColumnInfos, QStringList tableNames);
	virtual void beginTable(const QString& tableName, const QList<ExportColumnInfo>& columnInfos);
	virtual void beginRow();
	virtual void writeCell(const QVariant& value, const ExportColumnInfo& columnInfo);
	virtual void endRow();
	virtual void endTable();
	virtual void endExport();
};



#endif // EXPORT_WRITER_H
