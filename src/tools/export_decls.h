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
 * This file defines enums and structs related to data export.
 */

#ifndef EXPORT_DECLS_H
#define EXPORT_DECLS_H

#include "src/db/column.h"

#include <QDialog>
#include <QThread>
#include <QXmlStreamWriter>



/**
 * The mode in which to export the data.
 */
enum ExportMode
{
	OneTable,
	AsShown,
	Raw
};



/**
 * The file format in which to export the data.
 */
enum ExportFormat
{
	CSV,
	FODS
};



/**
 * A struct holding information about a single column included in the export.
 * 
 * The struct is deliberately not templated for Column or CompositeColumn to avoid the need for
 * a template parameter in the ExportWriter classes.
 */
struct ExportColumnInfo
{
	/** Whether the column is an export-only column, for which indices are handled differently. */
	bool exportOnly;
	/** The index of the column in the table, either in the order of normal columns or export-only columns. */
	int indexInTable;
	/** The translated UI name of the column. */
	QString name;
	/** The data type of the column. */
	DataType type;
	/** The list of translated names if the column uses an enum, or nullptr. */
	const QStringList* enumNames;
	/** The list of translated name lists if the column uses a dependant enum, or nullptr. */
	const QList<QPair<QString, QStringList>>* enumNameLists;
	
	/** A field for the FodsExportWriter to store the column's style name. */
	QString styleName = "Default";
};



#endif // EXPORT_DECLS_H
