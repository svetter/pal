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
 * @file export_writer.cpp
 * 
 * This file defines the ExportWriter class and its subclasses.
 */

#include "export_writer.h"

#include "export_thread.h"
#include "src/data/enum_names.h"

#include <QFileInfo>



/**
 * Creates a ExportWriter.
 * 
 * @param mode		The export mode.
 * @param filepath	The path and filename (or base filename if multiple files are written) to use
 * for the export.
 */
ExportWriter::ExportWriter(ExportMode mode, const QString& filepath) :
	mode(mode),
	filepath(filepath),
	file(nullptr)
{}

/**
 * Destroys the ExportWriter.
 */
ExportWriter::~ExportWriter()
{
	if (file) {
		file->close();
		delete file;
	}
}



/**
 * Writes an empty row, filling all cells with empty values.
 * 
 * @param columnInfos	ExportColumnInfo objects for all columns in the table.
 */
void ExportWriter::writeEmptyRow(const QList<ExportColumnInfo>& columnInfos)
{
	beginRow();
	for (int i = 0; i < columnInfos.size(); i++) {
		writeCell(QVariant(), columnInfos.at(i));
	}
	endRow();
}



/**
 * Opens a file for writing.
 * 
 * Handles errors by failing an assertion.
 * 
 * @param filepathToOpen	The path and filename to open.
 */
void ExportWriter::openFileForWriting(const QString& filepathToOpen)
{
	assert(!file);
	
	file = new QFile(filepathToOpen);
	
	bool success = file->open(QIODevice::WriteOnly);
	if (!success) {
		qDebug() << "File couldn't be opened:" << filepathToOpen;
		assert(success);
	}
}

/**
 * Inserts a string before the extension of a filepath.
 * 
 * @param baseFilepath	The base filepath.
 * @param insert		The string to insert before the extension.
 * @return				The filepath with the string inserted before the extension.
 */
QString ExportWriter::insertBeforeExtension(const QString& baseFilepath, const QString& insert)
{
	QFileInfo baseFile = QFileInfo(baseFilepath);
	QString path = baseFile.absolutePath();
	QString baseFilename = baseFile.fileName();
	
	QString extension = baseFile.suffix();
	if (!extension.isEmpty()) {
		baseFilename.chop(extension.size() + 1);
		extension = "." + extension;
	}
	
	return path + "/" + baseFilename + insert + extension;
}





/**
 * Creates a CsvExportWriter.
 * 
 * @param mode		The export mode.
 * @param filepath	The path and filename (or base filename if multiple files are written) to use for the export.
 * @param separator	The separator to use between columns.
 */
CsvExportWriter::CsvExportWriter(ExportMode mode, const QString& filepath, const QString& separator) :
	ExportWriter(mode, filepath),
	separator(separator),
	fileWriter(nullptr),
	numTables(-1),
	afterFirstItemInRow(false)
{
	assert(!separator.isNull());
	assert(separator.size() == 1);
}

/**
 * Destroys the CsvExportWriter.
 */
CsvExportWriter::~CsvExportWriter()
{
	if (fileWriter) {
		fileWriter->flush();
		delete fileWriter;
	}
}



/**
 * Called at the start of the entire export process.
 *
 * @param allColumnInfos	ExportColumnInfo objects for all columns in all tables.
 * @param tableNames		The translated UI names of all tables.
 */
void CsvExportWriter::beginExport(QList<QList<ExportColumnInfo>>& allColumnInfos, QStringList tableNames)
{
	Q_UNUSED(allColumnInfos);
	
	numTables = tableNames.size();
}

/**
 * Called at the start of a table during the export process.
 *
 * @param tableName		The translated UI name of the table.
 * @param columnInfos	ExportColumnInfo objects for all columns in the table.
 */
void CsvExportWriter::beginTable(const QString& tableName, const QList<ExportColumnInfo>& columnInfos)
{
	assert(numTables > 0);
	QString tableFilepath = filepath;
	if (numTables > 1) tableFilepath = insertBeforeExtension(filepath, "_" + tableName);
	
	openFileForWriting(tableFilepath);
	fileWriter = new QTextStream(file);
	
	bool firstInRow = true;
	for (const ExportColumnInfo& columnInfo : columnInfos) {
		if (!firstInRow) *fileWriter << separator;
		firstInRow = false;
		
		bool nameContainsSeparator = columnInfo.name.contains(separator);
		if (nameContainsSeparator) *fileWriter << "\"";
		*fileWriter << columnInfo.name;
		if (nameContainsSeparator) *fileWriter << "\"";
	}
}

/**
 * Called at the start of a row during the export process.
 */
void CsvExportWriter::beginRow()
{
	*fileWriter << Qt::endl;
	afterFirstItemInRow = false;
}

/**
 * Called for each cell during the export process.
 *
 * @param value			The value to write for the cell.
 * @param columnInfo	The ExportColumnInfo object for the column of the cell.
 */
void CsvExportWriter::writeCell(const QVariant& value, const ExportColumnInfo& columnInfo)
{
	if (afterFirstItemInRow) *fileWriter << separator;
	afterFirstItemInRow = true;
	
	if (!value.isValid() || value.isNull()) return;
	
	QString formattedValue;
	
	switch (columnInfo.type) {
	case String: {
		formattedValue = value.toString();
		break;
	}
	case Bit: {
		formattedValue = value.toBool() ? DataExportThread::tr("Yes") : DataExportThread::tr("No");
		break;
	}
	case Integer:
	case ID: {
		formattedValue = QString::number(value.toInt());
		break;
	}
	case Enum: {
		switch (mode) {
		case OneTable:
		case AsShown: {
			assert(columnInfo.enumNames);
			const QString& enumNameEnglish = columnInfo.enumNames->at(value.toInt());
			formattedValue = EnumNames::tr(enumNameEnglish.toStdString().c_str());
			break;
		}
		case Raw: {
			formattedValue = QString::number(value.toInt());
			break;
		}
		default: assert(false);
		}
		break;
	}
	case DualEnum: {
		switch (mode) {
		case OneTable:
		case AsShown: {
			// In composite columns, dual enums are lists of two connected indices
			QList<QVariant> list = value.toList();
			if (list.size() != 2)
			assert(list.size() == 2);
			assert(columnInfo.enumNameLists);
			const QString& enumNameEnglish = columnInfo.enumNameLists->at(list.at(0).toInt()).second.at(list.at(1).toInt());
			formattedValue = EnumNames::tr(enumNameEnglish.toStdString().c_str());
			break;
		}
		case Raw: {
			// In base tables, dual enums are still single indices
			formattedValue = QString::number(value.toInt());
			break;
		}
		default: assert(false);
		}
		break;
	}
	case Date: {
		formattedValue = value.toDate().toString("yyyy-MM-dd");
		break;
	}
	case Time: {
		formattedValue = value.toTime().toString("hh:mm");
		break;
	}
	default: assert(false);
	}
	if (formattedValue.isNull()) {
		qDebug() << "Warning: During CSV export, formatted null string from non-null QVariant:" << value;
	}
	
	bool formattedValueContainsSeparator = formattedValue.contains(separator);
	if (formattedValueContainsSeparator) *fileWriter << "\"";
	*fileWriter << formattedValue.replace("\"", "\\\"");
	if (formattedValueContainsSeparator) *fileWriter << "\"";
}

/**
 * Called at the end of a row during the export process.
 */
void CsvExportWriter::endRow()
{
	// Nothing to be done
}

/**
 * Called at the end of a table during the export process.
 */
void CsvExportWriter::endTable()
{
	fileWriter->flush();
	file->close();
	delete file;
	file = nullptr;
}

/**
 * Called at the end of the entire export process.
 */
void CsvExportWriter::endExport()
{
	// Nothing to be done
}





/**
 * Creates a FodsExportWriter.
 *
 * @param mode		The export mode.
 * @param filepath	The path and filename to use for the export.
 */
FodsExportWriter::FodsExportWriter(ExportMode mode, const QString& filepath) :
	ExportWriter(mode, filepath),
	xmlWriter(nullptr)
{}

/**
 * Destroys the CsvExportWriter.
 */
FodsExportWriter::~FodsExportWriter()
{
	if (xmlWriter) {
		xmlWriter->writeEndDocument();
		delete xmlWriter;
	}
}



/**
 * Called at the start of the entire export process.
 *
 * @param allColumnInfos	ExportColumnInfo objects for all columns in all tables.
 * @param tableNames		The translated UI names of all tables.
 */
void FodsExportWriter::beginExport(QList<QList<ExportColumnInfo>>& allColumnInfos, QStringList tableNames)
{
	openFileForWriting(filepath);
	
	xmlWriter = new QXmlStreamWriter(file);
	xmlWriter->setAutoFormatting(true);
	xmlWriter->setAutoFormattingIndent(-1);	// Indent with 1 tab
	
	xmlWriter->writeStartDocument();
	
	xmlWriter->writeStartElement("office:document"),
		xmlWriter->writeAttribute("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0"),
		xmlWriter->writeAttribute("xmlns:ooo", "http://openoffice.org/2004/office"),
		xmlWriter->writeAttribute("xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"),
		xmlWriter->writeAttribute("xmlns:config", "urn:oasis:names:tc:opendocument:xmlns:config:1.0"),
		xmlWriter->writeAttribute("xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0"),
		xmlWriter->writeAttribute("xmlns:text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0"),
		xmlWriter->writeAttribute("xmlns:table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0"),
		xmlWriter->writeAttribute("xmlns:number", "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0"),
		xmlWriter->writeAttribute("office:version", "1.3"),
		xmlWriter->writeAttribute("office:mimetype", "application/vnd.oasis.opendocument.spreadsheet");
	
	// Settings
	xmlWriter->writeStartElement("office:settings");
	xmlWriter->writeStartElement("config:config-item-set"),
		xmlWriter->writeAttribute("config:name", "ooo:view-settings");
	xmlWriter->writeStartElement("config:config-item-map-indexed"),
		xmlWriter->writeAttribute("config:name", "Views");
	xmlWriter->writeStartElement("config:config-item-map-entry");
	xmlWriter->writeStartElement("config:config-item"),
		xmlWriter->writeAttribute("config:name", "ViewId"),
		xmlWriter->writeAttribute("config:type", "string");
	xmlWriter->writeCharacters("view1");
	xmlWriter->writeEndElement();	// config:config-item
	
	// Table-wise settings
	xmlWriter->writeStartElement("config:config-item-map-named"),
		xmlWriter->writeAttribute("config:name", "Tables");
	for (QString& tableName : tableNames) {
		xmlWriter->writeStartElement("config:config-item-map-entry"),
			xmlWriter->writeAttribute("config:name", tableName);
		// Fix first row and column
		int fixRows = 1;
		int fixColumns = mode == Raw ? 2 : 1;
		QList<QStringList> attributes = {
			{"HorizontalSplitMode",		"short",	"2"},
			{"VerticalSplitMode",		"short",	"2"},
			{"HorizontalSplitPosition",	"int",		QString::number(fixColumns)},
			{"VerticalSplitPosition",	"int",		QString::number(fixRows)},
			{"PositionRight",			"int",		QString::number(fixColumns)},
			{"PositionBottom",			"int",		QString::number(fixRows)}
		};
		for (int i = 0; i < attributes.size(); i++) {
			xmlWriter->writeStartElement("config:config-item"),
				xmlWriter->writeAttribute("config:name", attributes.at(i).at(0)),
				xmlWriter->writeAttribute("config:type", attributes.at(i).at(1));
			xmlWriter->writeCharacters(attributes.at(i).at(2));
			xmlWriter->writeEndElement();	// config:config-item
		}
		xmlWriter->writeEndElement();	// config:config-item-map-entry
	}
	
	// End settings
	xmlWriter->writeEndElement();	// config:config-item-map-named
	xmlWriter->writeEndElement();	// config:config-item-map-entry
	xmlWriter->writeEndElement();	// config:config-item-map-indexed
	xmlWriter->writeEndElement();	// config:config-item-set
	xmlWriter->writeEndElement();	// office:settings
	
	// Styles
	xmlWriter->writeStartElement("office:automatic-styles");
	
	// Date style (N49)
	xmlWriter->writeStartElement("number:date-style"),
		xmlWriter->writeAttribute("style:name", "N49");
	xmlWriter->writeStartElement("number:year"),
		xmlWriter->writeAttribute("number:style", "long");
	xmlWriter->writeEndElement();	// number:year
	xmlWriter->writeTextElement("number:text", "-");
	xmlWriter->writeStartElement("number:month"),
		xmlWriter->writeAttribute("number:style", "long");
	xmlWriter->writeEndElement();	// number:month
	xmlWriter->writeTextElement("number:text", "-");
	xmlWriter->writeStartElement("number:day"),
		xmlWriter->writeAttribute("number:style", "long");
	xmlWriter->writeEndElement();	// number:day
	xmlWriter->writeEndElement();	// number:date-style
	
	// Time style (N60)
	xmlWriter->writeStartElement("number:time-style"),
		xmlWriter->writeAttribute("style:name", "N60");
	xmlWriter->writeStartElement("number:hours"),
		xmlWriter->writeAttribute("number:style", "long");
	xmlWriter->writeEndElement();	// number:hours
	xmlWriter->writeTextElement("number:text", ":");
	xmlWriter->writeStartElement("number:minutes"),
		xmlWriter->writeAttribute("number:style", "long");
	xmlWriter->writeEndElement();	// number:minutes
	xmlWriter->writeEndElement();	// number:time-style
	
	// Bold style
	xmlWriter->writeStartElement("style:style"),
		xmlWriter->writeAttribute("style:name", boldStyleName),
		xmlWriter->writeAttribute("style:family", "table-cell"),
		xmlWriter->writeAttribute("style:parent-style-name", "Default");
	xmlWriter->writeEmptyElement("style:text-properties");
		xmlWriter->writeAttribute("fo:font-weight", "bold"),
		xmlWriter->writeAttribute("style:font-weight-complex", "bold");
	xmlWriter->writeEndElement();	// style:style
	
	// Date style
	xmlWriter->writeStartElement("style:style"),
		xmlWriter->writeAttribute("style:name", dateStyleName),
		xmlWriter->writeAttribute("style:family", "table-cell"),
		xmlWriter->writeAttribute("style:parent-style-name", "Default"),
		xmlWriter->writeAttribute("style:data-style-name", "N49");
	xmlWriter->writeEndElement();	// style:style
	
	// Time style
	xmlWriter->writeStartElement("style:style"),
		xmlWriter->writeAttribute("style:name", timeStyleName),
		xmlWriter->writeAttribute("style:family", "table-cell"),
		xmlWriter->writeAttribute("style:parent-style-name", "Default"),
		xmlWriter->writeAttribute("style:data-style-name", "N60");
	xmlWriter->writeEndElement();	// style:style
	
	// Set column styles
	for (QList<ExportColumnInfo>& columnInfos : allColumnInfos) {
		for (ExportColumnInfo& columnInfo : columnInfos) {
			switch (columnInfo.type) {
			case String:
			case Bit:
			case Integer:
			case ID:
			case Enum:
			case DualEnum: {
				columnInfo.styleName = "Default";
				break;
			}
			case Date: {
				columnInfo.styleName = dateStyleName;
				break;
			}
			case Time: {
				columnInfo.styleName = timeStyleName;
				break;
			}
			default: assert(false);
			}
		}
	}
	
	xmlWriter->writeEndElement();	// office:automatic-styles
	
	// Begin body
	xmlWriter->writeStartElement("office:body");
	xmlWriter->writeStartElement("office:spreadsheet");
}

/**
 * Called at the start of a table during the export process.
 *
 * @param tableName		The translated UI name of the table.
 * @param columnInfos	ExportColumnInfo objects for all columns in the table.
 */
void FodsExportWriter::beginTable(const QString& tableName, const QList<ExportColumnInfo>& columnInfos)
{
	xmlWriter->writeStartElement("table:table"),
		xmlWriter->writeAttribute("table:name", tableName);
	
	// Columns styles
	for (const ExportColumnInfo& columnInfo : columnInfos) {
		xmlWriter->writeStartElement("table:table-column"),
			xmlWriter->writeAttribute("table:default-cell-style-name", columnInfo.styleName);
		xmlWriter->writeEndElement();	// table:table-column
	}
	
	// Write header row
	xmlWriter->writeStartElement("table:table-row");
	for (const ExportColumnInfo& columnInfo : columnInfos) {
		xmlWriter->writeStartElement("table:table-cell"),
			xmlWriter->writeAttribute("table:style-name", boldStyleName),
			xmlWriter->writeAttribute("office:value-type", "string");
		xmlWriter->writeStartElement("text:p");
		xmlWriter->writeCharacters(columnInfo.name);
		xmlWriter->writeEndElement();	// text:p
		xmlWriter->writeEndElement();	// table:table-cell
	}
	xmlWriter->writeEndElement();	// table:table-row
}

/**
 * Called at the start of a row during the export process.
 */
void FodsExportWriter::beginRow()
{
	xmlWriter->writeStartElement("table:table-row");
}

/**
 * Called for each cell during the export process.
 *
 * @param value			The value to write for the cell.
 * @param columnInfo	The ExportColumnInfo object for the column of the cell.
 */
void FodsExportWriter::writeCell(const QVariant& value, const ExportColumnInfo& columnInfo)
{
	if (!value.isValid() || value.isNull()) {
		xmlWriter->writeEmptyElement("table:table-cell");
		return;
	}
	
	xmlWriter->writeStartElement("table:table-cell");
	
	QString formattedValue;
	
	switch (columnInfo.type) {
	case String: {
		formattedValue = value.toString();
		xmlWriter->writeAttribute("office:value-type", "string");
		break;
	}
	case Bit: {
		formattedValue = value.toBool() ? DataExportThread::tr("Yes") : DataExportThread::tr("No");
		xmlWriter->writeAttribute("office:value-type", "string");
		break;
	}
	case Integer:
	case ID: {
		formattedValue = QString::number(value.toInt());
		xmlWriter->writeAttribute("office:value-type", "float");
		xmlWriter->writeAttribute("office:value", formattedValue);
		break;
	}
	case Enum: {
		switch (mode) {
		case OneTable:
		case AsShown: {
			assert(columnInfo.enumNames);
			formattedValue = columnInfo.enumNames->at(value.toInt());
			xmlWriter->writeAttribute("office:value-type", "string");
			break;
		}
		case Raw: {
			formattedValue = QString::number(value.toInt());
			xmlWriter->writeAttribute("office:value-type", "float");
			xmlWriter->writeAttribute("office:value", formattedValue);
			break;
		}
		default: assert(false);
		}
		break;
	}
	case DualEnum: {
		switch (mode) {
		case OneTable:
		case AsShown: {
			// In composite columns, dual enums are lists of two connected indices
			if (value.isValid()) {
				QList<QVariant> list = value.toList();
				assert(list.size() == 2);
				assert(columnInfo.enumNameLists);
				formattedValue = columnInfo.enumNameLists->at(list.at(0).toInt()).second.at(list.at(1).toInt());
			}
			xmlWriter->writeAttribute("office:value-type", "string");
			break;
		}
		case Raw: {
			// In base tables, dual enums are still single indices
			formattedValue = QString::number(value.toInt());
			xmlWriter->writeAttribute("office:value-type", "float");
			xmlWriter->writeAttribute("office:value", formattedValue);
			break;
		}
		default: assert(false);
		}
		break;
	}
	case Date: {
		formattedValue = value.toDate().toString("yyyy-MM-dd");
		xmlWriter->writeAttribute("office:value-type", "date"),
		xmlWriter->writeAttribute("office:date-value", formattedValue);
		break;
	}
	case Time: {
		formattedValue = value.toTime().toString("hh:mm");
		QString encodedValue = value.toTime().toString("'PT'hh'H'mm'M'ss'S'");
		xmlWriter->writeAttribute("office:value-type", "time");
		xmlWriter->writeAttribute("office:time-value", encodedValue);
		break;
	}
	default: assert(false);
	}
	assert(!formattedValue.isNull());
	
	xmlWriter->writeStartElement("text:p");
	xmlWriter->writeCharacters(formattedValue);
	xmlWriter->writeEndElement();	// text:p
	xmlWriter->writeEndElement();	// table:table-cell
}

/**
 * Called at the end of a row during the export process.
 */
void FodsExportWriter::endRow()
{
	xmlWriter->writeEndElement();	// table:table-row
}

/**
 * Called at the end of a table during the export process.
 */
void FodsExportWriter::endTable()
{
	xmlWriter->writeEndElement();	// table:table
}

/**
 * Called at the end of the entire export process.
 */
void FodsExportWriter::endExport()
{
	xmlWriter->writeEndElement();	// office:spreadsheet
	xmlWriter->writeEndElement();	// office:body
	xmlWriter->writeEndElement();	// office:document
	
	delete xmlWriter;
	xmlWriter = nullptr;
	file->close();
	delete file;
	file = nullptr;
}
