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
 * @file export_thread.cpp
 * 
 * This file defines the DataExportThread class.
 */

#include "export_thread.h"



DataExportThread::DataExportThread(QDialog* parent, Database* db, const ItemTypesHandler* typesHandler, ExportMode mode, bool includeStats, const QString& filepath, ExportFormat format, QString csvSeparator) :
		QThread(parent),
		db(db),
		typesHandler(typesHandler),
		mode(mode),
		includeStats(includeStats),
		filepath(filepath),
		format(format),
		csvSeparator(csvSeparator),
		workloadSize(-1),
		abortWasCalled(false),
		file(nullptr),
		fileWriter(nullptr),
		xmlWriter(nullptr)
{}

DataExportThread::~DataExportThread()
{
	if (xmlWriter) {
		xmlWriter->writeEndDocument();
		delete xmlWriter;
	}
	if (fileWriter) {
		fileWriter->flush();
		delete fileWriter;
	}
	if (file) {
		file->close();
		delete file;
	}
}



void DataExportThread::run()
{
	abortWasCalled = false;
	
	switch (mode) {
	case Raw: {
		// Export ascents table with references pulled in
		exportRaw();
		break;
	}
	case Readable: {
		// Export tables separately
		exportReadable();
		break;
	}
	default: assert(false);
	}
}

void DataExportThread::abort() {
	abortWasCalled = true;
}



void DataExportThread::exportRaw()
{
	// Assemble column info lists
	const QList<Table*> baseTables = typesHandler->getAllBaseTables();
	QStringList tableNames = QStringList();
	QList<QList<ExportColumnInfo>> allColumnInfos = QList<QList<ExportColumnInfo>>();
	for (const Table* const baseTable : baseTables) {
		QList<ExportColumnInfo> columnInfos = QList<ExportColumnInfo>();
		for (const Column* column : baseTable->getColumnList()) {
			columnInfos.append({column->getIndex(), column->uiName, column->type, column->enumNames, column->enumNameLists});
		}
		tableNames.append(baseTable->uiName);
		allColumnInfos.append(columnInfos);
	}
	
	// Determine and report workload size
	workloadSize = 0;	// In cells
	for (int tableIndex = 0; tableIndex < baseTables.size(); tableIndex++) {
		const Table* const baseTable = baseTables.at(tableIndex);
		workloadSize += baseTable->getNumberOfRows() * allColumnInfos.at(tableIndex).size();
	}
	emit callback_reportWorkloadSize(workloadSize);
	int progress = 0;
	
	
	// Start writing file(s)
	beginExport(allColumnInfos, tableNames);
	
	// FOR ALL BASE TABLES
	for (int tableIndex = 0; tableIndex < baseTables.size(); tableIndex++) {
		const Table* const baseTable = baseTables.at(tableIndex);
		const QList<ExportColumnInfo>& columnInfos = allColumnInfos.at(tableIndex);
		
		emit callback_setProgressText(tr("Exporting table %1...").arg(baseTable->uiName));
		beginTable(baseTable->uiName, columnInfos);
		
		// FOR ALL ROWS IN TABLE
		int numRows = baseTable->getNumberOfRows();
		for (BufferRowIndex rowIndex = BufferRowIndex(0); rowIndex.isValid(numRows); rowIndex++) {
			beginRow();
			
			// FOR ALL CELLS IN ROW
			bool firstInRow = true;
			for (const ExportColumnInfo& columnInfo : columnInfos) {
				QVariant value = baseTable->getColumnByIndex(columnInfo.indexInTable)->getValueAt(rowIndex);
				writeCell(value, columnInfo, firstInRow);
				
				emit callback_reportProgress(++progress);
				firstInRow = false;
			}
			
			endRow();
			if (abortWasCalled) break;
		}
		
		endTable();
		if (abortWasCalled) break;
	}
	
	// Write enum names table
	writeEnumTable(baseTables, allColumnInfos);
	
	endExport();
}


void DataExportThread::writeEnumTable(const QList<Table*>& baseTables, const QList<QList<ExportColumnInfo>>& allColumnInfos)
{
	// Structure: Table name - column name - [discerning index - discerning name - ] enum index - enum name
	QList<ExportColumnInfo> enumTableColumnInfos = QList<ExportColumnInfo>();
	enumTableColumnInfos.append({0,	tr("Table"),					String,		nullptr,	nullptr});
	enumTableColumnInfos.append({1,	tr("Column"),					String,		nullptr,	nullptr});
	enumTableColumnInfos.append({2,	tr("Enumerator index"),			Integer,	nullptr,	nullptr});
	enumTableColumnInfos.append({3,	tr("Enumerator name"),			String,		nullptr,	nullptr});
	enumTableColumnInfos.append({4,	tr("Nested enumerator index"),	Integer,	nullptr,	nullptr});
	enumTableColumnInfos.append({5,	tr("Nested enumerator name"),	String,		nullptr,	nullptr});
	
	beginTable(tr("Enumerator names"), enumTableColumnInfos);
	emit callback_setProgressText(tr("Exporting enumerator names table..."));
	
	for (int tableIndex = 0; tableIndex < baseTables.size(); tableIndex++) {
		const Table* const baseTable = baseTables.at(tableIndex);
		const QList<ExportColumnInfo>& columnInfos = allColumnInfos.at(tableIndex);
		for (const ExportColumnInfo& columnInfo : columnInfos) {
			if (columnInfo.enumNames) {
				writeEmptyRow(columnInfos);
				for (int i = 0; i < columnInfo.enumNames->size(); i++) {
					beginRow();
					writeCell(baseTable->uiName,			enumTableColumnInfos.at(0),	true);
					writeCell(columnInfo.name,				enumTableColumnInfos.at(1),	false);
					writeCell(i,							enumTableColumnInfos.at(2),	false);
					writeCell(columnInfo.enumNames->at(i),	enumTableColumnInfos.at(3),	false);
					writeCell(QVariant(),					enumTableColumnInfos.at(4),	false);
					writeCell(QVariant(),					enumTableColumnInfos.at(5),	false);
					endRow();
				}
			}
			
			if (columnInfo.enumNameLists) {
				for (int j = 0; j < columnInfo.enumNameLists->size(); j++) {
					writeEmptyRow(columnInfos);
					const QString& discerningEnumName = columnInfo.enumNameLists->at(j).first;
					const QStringList& enumNames = columnInfo.enumNameLists->at(j).second;
					for (int i = 0; i < columnInfo.enumNames->size(); i++) {
						beginRow();
						writeCell(baseTable->uiName,	enumTableColumnInfos.at(0),	true);
						writeCell(columnInfo.name,		enumTableColumnInfos.at(1),	false);
						writeCell(j,					enumTableColumnInfos.at(2),	false);
						writeCell(discerningEnumName,	enumTableColumnInfos.at(3),	false);
						writeCell(i,					enumTableColumnInfos.at(4),	false);
						writeCell(enumNames.at(i),		enumTableColumnInfos.at(5),	false);
						endRow();
					}
				}
			}
		}
	}
	
	endTable();
}



void DataExportThread::exportReadable()
{
	// Assemble column info lists
	QList<CompositeTable*> compTables = QList<CompositeTable*>();
	QStringList tableNames = QStringList();
	QList<QList<ExportColumnInfo>> allColumnInfos = QList<QList<ExportColumnInfo>>();
	typesHandler->forEach([&] (ItemTypeMapper& mapper) {
		QList<ExportColumnInfo> columnInfos = QList<ExportColumnInfo>();
		for (const CompositeColumn* column : mapper.compTable->getColumnList()) {
			// Filter columns
			bool skipColumn = false;
			skipColumn |= column->isBackendColumn();
			skipColumn |= !includeStats && column->isStatistical;
			if (skipColumn) continue;
			
			columnInfos.append({column->getIndex(), column->uiName, column->contentType, column->enumNames, column->enumNameLists});
		}
		
		compTables.append(mapper.compTable);
		tableNames.append(mapper.compTable->name);
		allColumnInfos.append(columnInfos);
	});
	
	// Determine and report workload size
	int workloadSize = 0;	// In cells
	for (int tableIndex = 0; tableIndex < compTables.size(); tableIndex++) {
		const CompositeTable* const compTable = compTables.at(tableIndex);
		workloadSize += compTable->getNumberOfCellsToUpdate();
		workloadSize += compTable->rowCount() * allColumnInfos.at(tableIndex).size();
	}
	emit callback_reportWorkloadSize(workloadSize);
	int progress = 0;
	
	// Make sure all tables are up to date
	for (CompositeTable* const compTable : compTables) {
		if (compTable->getNumberOfCellsToUpdate() < 1) continue;
		
		emit callback_setProgressText(tr("Preparing table %1...").arg(compTable->name));
		auto progressLambda = [this, &progress] () {
			emit callback_reportProgress(++progress);
		};
		compTable->updateBuffer(progressLambda);
	}
	
	
	// Start writing file(s)
	beginExport(allColumnInfos, tableNames);
	
	// FOR ALL COMPOSITE TABLES
	for (int tableIndex = 0; tableIndex < compTables.size(); tableIndex++) {
		const CompositeTable* const compTable = compTables.at(tableIndex);
		const QList<ExportColumnInfo>& columnInfos = allColumnInfos.at(tableIndex);
		
		emit callback_setProgressText(tr("Exporting table %1...").arg(compTable->name));
		beginTable(compTable->name, columnInfos);
		
		// FOR ALL ROWS IN TABLE
		int numRows = compTable->rowCount();
		for (BufferRowIndex rowIndex = BufferRowIndex(0); rowIndex.isValid(numRows); rowIndex++) {
			beginRow();
			
			// FOR ALL CELLS IN ROW
			bool firstInRow = true;
			for (const ExportColumnInfo& columnInfo : columnInfos) {
				QVariant value = compTable->getColumnAt(columnInfo.indexInTable)->getRawValueAt(rowIndex);
				writeCell(value, columnInfo, firstInRow);
				
				emit callback_reportProgress(++progress);
				firstInRow = false;
			}
			
			endRow();
			if (abortWasCalled) break;
		}
		
		endTable();
		if (abortWasCalled) break;
	}
	
	endExport();
}



void DataExportThread::openFileForWriting(const QString& filepathToOpen)
{
	assert(!file);
	
	file = new QFile(filepathToOpen);
	
	if (!file->open(QIODevice::WriteOnly)) {
		qDebug() << "File couldn't be opened";
		abort();
	}
}



void DataExportThread::beginExport(QList<QList<ExportColumnInfo>>& allColumnInfos, QStringList tableNames)
{
	switch (format) {
	case CSV:	return;
	case FODS:	return beginExportFODS(allColumnInfos, tableNames);
	}
}

void DataExportThread::beginTable(const QString& tableName, const QList<ExportColumnInfo>& columnInfos)
{
	switch (format) {
	case CSV:	return beginTableCSV(tableName, columnInfos);
	case FODS:	return beginTableFODS(tableName, columnInfos);
	}
}

void DataExportThread::beginRow()
{
	switch (format) {
	case CSV:	return beginRowCSV();
	case FODS:	return beginRowFODS();
	}
}

void DataExportThread::writeCell(const QVariant& value, const ExportColumnInfo& columnInfo, bool firstInRow)
{
	switch (format) {
	case CSV:	return writeCellCSV(value, columnInfo, firstInRow);
	case FODS:	return writeCellFODS(value, columnInfo);
	}
}

void DataExportThread::endRow()
{
	switch (format) {
	case CSV:	return;
	case FODS:	return endRowFODS();
	}
}

void DataExportThread::writeEmptyRow(const QList<ExportColumnInfo>& columnInfos)
{
	beginRow();
	for (int i = 0; i < columnInfos.size(); i++) {
		writeCell(QVariant(), columnInfos.at(i), i == 0);
	}
	endRow();
}

void DataExportThread::endTable()
{
	switch (format) {
	case CSV:	return endTableCSV();
	case FODS:	return endTableFODS();
	}
}

void DataExportThread::endExport()
{
	switch (format) {
	case CSV:	return;
	case FODS:	return endExportFODS();
	}
}



void DataExportThread::beginTableCSV(const QString& tableName, const QList<ExportColumnInfo>& columnInfos)
{
	assert(!csvSeparator.isNull());
	openFileForWriting(insertBeforeExtension(filepath, "_" + tableName));
	fileWriter = new QTextStream(file);
	
	bool firstInRow = true;
	for (const ExportColumnInfo& columnInfo : columnInfos) {
		if (!firstInRow) *fileWriter << csvSeparator;
		firstInRow = false;
		
		bool nameContainsSeparator = columnInfo.name.contains(csvSeparator);
		if (nameContainsSeparator) *fileWriter << "\"";
		*fileWriter << columnInfo.name;
		if (nameContainsSeparator) *fileWriter << "\"";
	}
}

void DataExportThread::beginRowCSV()
{
	*fileWriter << Qt::endl;
}

void DataExportThread::writeCellCSV(const QVariant& value, const ExportColumnInfo& columnInfo, bool firstInRow)
{
	if (!firstInRow) *fileWriter << csvSeparator;
	
	if (!value.isValid() || value.isNull()) return;
	
	QString formattedValue;
	
	switch (columnInfo.type) {
	case String: {
		formattedValue = value.toString();
		break;
	}
	case Bit: {
		formattedValue = value.toBool() ? tr("Yes") : tr("No");
		break;
	}
	case Integer:
	case ID: {
		formattedValue = QString::number(value.toInt());
		break;
	}
	case IDList: {
		QList<QVariant> idList = value.toList();
		formattedValue = "";
		for (const QVariant& id : idList) {
			if (!formattedValue.isEmpty()) formattedValue += ", ";
			formattedValue += QString::number(id.toInt());
		}
		break;
	}
	case Enum: {
		switch (mode) {
		case Raw: {
			formattedValue = QString::number(value.toInt());
			break;
		}
		case Readable: {
			assert(columnInfo.enumNames);
			const QString& enumNameEnglish = columnInfo.enumNames->at(value.toInt());
			formattedValue = EnumNames::tr(enumNameEnglish.toStdString().c_str());
			break;
		}
		default: assert(false);
		}
		break;
	}
	case DualEnum: {
		switch (mode) {
		case Raw: {
			// In base tables, dual enums are still single indices
			formattedValue = QString::number(value.toInt());
			break;
		}
		case Readable: {
			// In composite columns, dual enums are lists of two connected indices
			QList<QVariant> list = value.toList();
			if (list.size() != 2)
			assert(list.size() == 2);
			assert(columnInfo.enumNameLists);
			const QString& enumNameEnglish = columnInfo.enumNameLists->at(list.at(0).toInt()).second.at(list.at(1).toInt());
			formattedValue = EnumNames::tr(enumNameEnglish.toStdString().c_str());
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
	assert(!formattedValue.isNull());
	
	bool formattedValueContainsSeparator = formattedValue.contains(csvSeparator);
	if (formattedValueContainsSeparator) *fileWriter << "\"";
	*fileWriter << formattedValue.replace("\"", "\\\"");
	if (formattedValueContainsSeparator) *fileWriter << "\"";
}

void DataExportThread::endTableCSV()
{
	fileWriter->flush();
	file->close();
	delete file;
	file = nullptr;
}



void DataExportThread::beginExportFODS(QList<QList<ExportColumnInfo>>& allColumnInfos, QStringList tableNames)
{
	openFileForWriting(filepath);
	
	xmlWriter = new QXmlStreamWriter(file);
	xmlWriter->setAutoFormatting(true);
	xmlWriter->setAutoFormattingIndent(-1);	// Indent with 1 tab
	
	xmlWriter->writeStartDocument();
	
	xmlWriter->writeStartElement("office:document"),
		xmlWriter->writeAttribute("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0"),
		xmlWriter->writeAttribute("xmlns:ooo", "http://openoffice.org/2004/office"),
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
		xmlWriter->writeAttribute("style:name", fods_boldStyleName),
		xmlWriter->writeAttribute("style:family", "table-cell"),
		xmlWriter->writeAttribute("style:parent-style-name", "Default");
	xmlWriter->writeEmptyElement("style:text-properties");
		xmlWriter->writeAttribute("fo:font-weight", "bold"),
		xmlWriter->writeAttribute("style:font-weight-complex", "bold");
	xmlWriter->writeEndElement();	// style:style
	
	// Date style
	xmlWriter->writeStartElement("style:style"),
		xmlWriter->writeAttribute("style:name", fods_dateStyleName),
		xmlWriter->writeAttribute("style:family", "table-cell"),
		xmlWriter->writeAttribute("style:parent-style-name", "Default"),
		xmlWriter->writeAttribute("style:data-style-name", "N49");
	xmlWriter->writeEndElement();	// style:style
	
	// Time style
	xmlWriter->writeStartElement("style:style"),
		xmlWriter->writeAttribute("style:name", fods_timeStyleName),
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
			case IDList:
			case Enum:
			case DualEnum: {
				columnInfo.styleName = "Default";
				break;
			}
			case Date: {
				columnInfo.styleName = fods_dateStyleName;
				break;
			}
			case Time: {
				columnInfo.styleName = fods_timeStyleName;
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

void DataExportThread::beginTableFODS(const QString& tableName, const QList<ExportColumnInfo>& columnInfos)
{
	// Write the table:table element
	xmlWriter->writeStartElement("table:table"),
		xmlWriter->writeAttribute("table:name", tableName);
	
	for (const ExportColumnInfo& columnInfo : columnInfos) {
		// Write the table:table-column elements
		xmlWriter->writeStartElement("table:table-column"),
			xmlWriter->writeAttribute("table:default-cell-style-name", columnInfo.styleName);
		xmlWriter->writeEndElement();	// table:table-column
	}
	
	// Write header row
	xmlWriter->writeStartElement("table:table-row");
	for (const ExportColumnInfo& columnInfo : columnInfos) {
		xmlWriter->writeStartElement("table:table-cell"),
			xmlWriter->writeAttribute("table:style-name", fods_boldStyleName),
			xmlWriter->writeAttribute("office:value-type", "string");
		xmlWriter->writeStartElement("text:p");
		xmlWriter->writeCharacters(columnInfo.name);
		xmlWriter->writeEndElement();	// text:p
		xmlWriter->writeEndElement();	// table:table-cell
	}
	xmlWriter->writeEndElement();	// table:table-row
}

void DataExportThread::beginRowFODS()
{
	xmlWriter->writeStartElement("table:table-row");
}

void DataExportThread::writeCellFODS(const QVariant& value, const ExportColumnInfo& columnInfo)
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
		formattedValue = value.toBool() ? tr("Yes") : tr("No");
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
	case IDList: {
		QList<QVariant> idList = value.toList();
		formattedValue = "";
		for (const QVariant& id : idList) {
			if (!formattedValue.isEmpty()) formattedValue += ", ";
			formattedValue += QString::number(id.toInt());
		}
		xmlWriter->writeAttribute("office:value-type", "string");
		break;
	}
	case Enum: {
		switch (mode) {
		case Raw: {
			formattedValue = QString::number(value.toInt());
			xmlWriter->writeAttribute("office:value-type", "float");
			xmlWriter->writeAttribute("office:value", formattedValue);
			break;
		}
		case Readable: {
			assert(columnInfo.enumNames);
			formattedValue = columnInfo.enumNames->at(value.toInt());
			xmlWriter->writeAttribute("office:value-type", "string");
			break;
		}
		default: assert(false);
		}
		break;
	}
	case DualEnum: {
		switch (mode) {
		case Raw: {
			// In base tables, dual enums are still single indices
			formattedValue = QString::number(value.toInt());
			xmlWriter->writeAttribute("office:value-type", "float");
			xmlWriter->writeAttribute("office:value", formattedValue);
			break;
		}
		case Readable: {
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
		if (!value.isValid()) 
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

void DataExportThread::endRowFODS()
{
	xmlWriter->writeEndElement();	// table:table-row
}

void DataExportThread::endTableFODS()
{
	xmlWriter->writeEndElement();	// table:table
}

void DataExportThread::endExportFODS()
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



QString DataExportThread::insertBeforeExtension(const QString& baseFilepath, const QString& insert)
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
