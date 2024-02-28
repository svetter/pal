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
 * @file export_thread.cpp
 * 
 * This file defines the DataExportThread class.
 */

#include "export_thread.h"



/**
 * Creates a new DataExportThread.
 * 
 * Creates the format-specific export writer.
 * 
 * @param parent		The parent window.
 * @param typesHandler	The project types handler.
 * @param mode			The export mode.
 * @param includeStats	Whether to include statistical columns.
 * @param filepath		The path to the file to export to, or path and base filename if multiple files are created.
 * @param format		The export file format.
 * @param csvSeparator	The CSV separator to use, if applicable.
 */
DataExportThread::DataExportThread(QDialog& parent, const ItemTypesHandler& typesHandler, ExportMode mode, bool includeStats, const QString& filepath, ExportFormat format, QString csvSeparator) :
	QThread(&parent),
	typesHandler(typesHandler),
	mode(mode),
	includeStats(includeStats),
	filepath(filepath),
	writer(nullptr),
	workloadSize(-1),
	abortWasCalled(false)
{
	switch (format) {
	case CSV: {
		writer = new CsvExportWriter(mode, filepath, csvSeparator);
		break;
	}
	case FODS: {
		writer = new FodsExportWriter(mode, filepath);
		break;
	}
	default: assert(false);
	}
}

/**
 * Destroys the DataExportThread.
 */
DataExportThread::~DataExportThread()
{
	if (writer) delete writer;
}



/**
 * Starts the export.
 * 
 * Delegates to the appropriate export method.
 */
void DataExportThread::run()
{
	abortWasCalled = false;
	
	switch (mode) {
	case OneTable: {
		// Export data consolidated into ascents table
		exportOneTable();
		break;
	}
	case AsShown: {
		// Export tables separately
		exportAsShown();
		break;
	}
	case Raw: {
		// Export ascents table with references pulled in
		exportRaw();
		break;
	}
	default: assert(false);
	}
}

/**
 * Gracefully aborts the thread.
 */
void DataExportThread::abort() {
	abortWasCalled = true;
}



/**
 * Performs the export for the OneTable mode.
 * 
 * In this mode, an extended version of the Ascents table is exported. The added columns contain
 * data which is not shown in the UI ascents table, and would otherwise be missing from the export.
 * However, this mode still does not export all data present in the database.
 */
void DataExportThread::exportOneTable()
{
	// Assemble column info list
	const ItemTypeMapper& mapper = typesHandler.get(ItemTypeAscent);
	CompositeTable& compTable = *mapper.compTable;
	QList<QList<ExportColumnInfo>> allColumnInfos = QList<QList<ExportColumnInfo>>();
	{
		QList<ExportColumnInfo> columnInfos = QList<ExportColumnInfo>();
		for (const CompositeColumn* column : mapper.compTable->getCompleteExportColumnList()) {
			// Determine whether to skip this column
			bool skipColumn = false;
			skipColumn |= column->isFilterOnlyColumn();
			if (skipColumn) continue;
			
			bool exportOnlyColumn = column->isExportOnlyColumn();
			int index = exportOnlyColumn ? column->getExportIndex() : column->getIndex();
			assert(index >= 0);
			
			columnInfos.append({exportOnlyColumn, index, column->uiName, column->contentType, column->enumNames, column->enumNameLists});
		}
		allColumnInfos.append(columnInfos);
	}
	
	// Determine and report workload size
	int workloadSize = 0;	// In cells
	workloadSize += compTable.getNumberOfCellsToUpdate();
	workloadSize += compTable.rowCount() * allColumnInfos.at(0).size();
	emit callback_reportWorkloadSize(workloadSize);
	int progress = 0;
	
	// Make sure table is up to date
	if (compTable.getNumberOfCellsToUpdate() >= 1) {
		emit callback_setProgressText(tr("Preparing table..."));
		auto progressLambda = [this, &progress] () {
			emit callback_reportProgress(++progress);
		};
		compTable.updateBothBuffers(progressLambda);
	}
	
	
	// Start writing file(s)
	writer->beginExport(allColumnInfos, { compTable.uiName });
	
	emit callback_setProgressText(tr("Exporting table..."));
	writer->beginTable(compTable.uiName, allColumnInfos.at(0));
	
	// FOR ALL ROWS IN TABLE
	int numRows = compTable.rowCount();
	for (BufferRowIndex rowIndex = BufferRowIndex(0); rowIndex.isValid(numRows); rowIndex++) {
		if (abortWasCalled) break;
		
		writer->beginRow();
		
		// FOR ALL CELLS IN ROW
		for (const ExportColumnInfo& columnInfo : allColumnInfos.at(0)) {
			QVariant value;
			if (columnInfo.exportOnly) {
				value = compTable.getExportOnlyColumnAt(columnInfo.indexInTable).computeValueAt(rowIndex);
			} else {
				value = compTable.getColumnAt(columnInfo.indexInTable).getRawValueAt(rowIndex);
			}
			
			writer->writeCell(value, columnInfo);
			
			emit callback_reportProgress(++progress);
		}
		
		writer->endRow();
	}
	
	writer->endTable();
	writer->endExport();
}



/**
 * Performs the export for the AsShown mode.
 * 
 * In this mode, each table is exported separately, with only the columns shown in the UI. The
 * additional option whether to include statistical columns is used to filter certain columns
 * according to a flag in CompositeColumn. Regardless of this option, the export will not contain
 * all data present in the database.
 */
void DataExportThread::exportAsShown()
{
	// Assemble column info lists
	QList<CompositeTable*> compTables = QList<CompositeTable*>();
	QStringList tableNames = QStringList();
	QList<QList<ExportColumnInfo>> allColumnInfos = QList<QList<ExportColumnInfo>>();
	for (const ItemTypeMapper* const mapper : typesHandler.getAllMappers()) {
		QList<ExportColumnInfo> columnInfos = QList<ExportColumnInfo>();
		for (const CompositeColumn* column : mapper->compTable->getNormalColumnList()) {
			bool skipColumn = !includeStats && column->isStatistical;
			if (skipColumn) continue;
			
			columnInfos.append({false, column->getIndex(), column->uiName, column->contentType, column->enumNames, column->enumNameLists});
		}
		
		compTables.append(mapper->compTable);
		tableNames.append(mapper->compTable->uiName);
		allColumnInfos.append(columnInfos);
	}
	
	// Determine and report workload size
	int workloadSize = 0;	// In cells
	for (int tableIndex = 0; tableIndex < compTables.size(); tableIndex++) {
		const CompositeTable& compTable = *compTables.at(tableIndex);
		workloadSize += compTable.getNumberOfCellsToUpdate();
		workloadSize += compTable.rowCount() * allColumnInfos.at(tableIndex).size();
	}
	emit callback_reportWorkloadSize(workloadSize);
	int progress = 0;
	
	// Make sure all tables are up to date
	for (CompositeTable* const compTable : compTables) {
		if (abortWasCalled) break;
		if (compTable->getNumberOfCellsToUpdate() < 1) continue;
		
		emit callback_setProgressText(tr("Preparing table %1...").arg(compTable->uiName));
		auto progressLambda = [this, &progress] () {
			emit callback_reportProgress(++progress);
		};
		compTable->updateBothBuffers(progressLambda);
	}
	
	
	// Start writing file(s)
	writer->beginExport(allColumnInfos, tableNames);
	
	// FOR ALL COMPOSITE TABLES
	for (int tableIndex = 0; tableIndex < compTables.size(); tableIndex++) {
		if (abortWasCalled) break;
		
		const CompositeTable& compTable = *compTables.at(tableIndex);
		const QList<ExportColumnInfo>& columnInfos = allColumnInfos.at(tableIndex);
		
		emit callback_setProgressText(tr("Exporting table %1...").arg(compTable.uiName));
		writer->beginTable(compTable.uiName, columnInfos);
		
		// FOR ALL ROWS IN TABLE
		int numRows = compTable.rowCount();
		for (BufferRowIndex rowIndex = BufferRowIndex(0); rowIndex.isValid(numRows); rowIndex++) {
			if (abortWasCalled) break;
			
			writer->beginRow();
			
			// FOR ALL CELLS IN ROW
			for (const ExportColumnInfo& columnInfo : columnInfos) {
				QVariant value = compTable.getColumnAt(columnInfo.indexInTable).getRawValueAt(rowIndex);
				writer->writeCell(value, columnInfo);
				
				emit callback_reportProgress(++progress);
			}
			
			writer->endRow();
		}
		
		writer->endTable();
	}
	
	writer->endExport();
}



/**
 * Performs the export for the Raw mode.
 * 
 * In this mode, all tables in the database itself are exported as they are (as opposed to the
 * tables shown to the user in the UI). This mode is the only one which exports all data present
 * in the database.
 * To enable decoding of enum values, an additional table is exported which lists the names of all
 * enums used in the database, paired with their indices.
 */
void DataExportThread::exportRaw()
{
	// Assemble column info lists
	const QList<Table*> baseTables = typesHandler.getAllBaseTables();
	QStringList tableNames = QStringList();
	QList<QList<ExportColumnInfo>> allColumnInfos = QList<QList<ExportColumnInfo>>();
	for (const Table* const baseTable : baseTables) {
		QList<ExportColumnInfo> columnInfos = QList<ExportColumnInfo>();
		for (const Column* column : baseTable->getColumnList()) {
			columnInfos.append({false, column->getIndex(), column->uiName, column->type, column->enumNames, column->enumNameLists});
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
	writer->beginExport(allColumnInfos, tableNames);
	
	// FOR ALL BASE TABLES
	for (int tableIndex = 0; tableIndex < baseTables.size(); tableIndex++) {
		if (abortWasCalled) break;
		
		const Table* const baseTable = baseTables.at(tableIndex);
		const QList<ExportColumnInfo>& columnInfos = allColumnInfos.at(tableIndex);
		
		emit callback_setProgressText(tr("Exporting table %1...").arg(baseTable->uiName));
		writer->beginTable(baseTable->uiName, columnInfos);
		
		// FOR ALL ROWS IN TABLE
		int numRows = baseTable->getNumberOfRows();
		for (BufferRowIndex rowIndex = BufferRowIndex(0); rowIndex.isValid(numRows); rowIndex++) {
			if (abortWasCalled) break;
			
			writer->beginRow();
			
			// FOR ALL CELLS IN ROW
			for (const ExportColumnInfo& columnInfo : columnInfos) {
				QVariant value = baseTable->getColumnByIndex(columnInfo.indexInTable).getValueAt(rowIndex);
				writer->writeCell(value, columnInfo);
				
				emit callback_reportProgress(++progress);
			}
			
			writer->endRow();
		}
		
		writer->endTable();
	}
	
	// Write enum names table
	writeEnumTable(baseTables, allColumnInfos);
	
	writer->endExport();
}


/**
 * For the Raw export mode, writes the table containing the names of all enums used in the database.
 * 
 * Automatically collects all enum name lists from the column info lists and writes them to a
 * dedicated table.
 * 
 * @param baseTables		The list of base tables.
 * @param allColumnInfos	The list of column info lists for the whole raw export.
 */
void DataExportThread::writeEnumTable(const QList<Table*>& baseTables, const QList<QList<ExportColumnInfo>>& allColumnInfos)
{
	// Structure: Table name - column name - [discerning index - discerning name - ] enum index - enum name
	QList<ExportColumnInfo> enumTableColumnInfos = QList<ExportColumnInfo>();
	enumTableColumnInfos.append({false,	0,	tr("Table"),					String,		nullptr,	nullptr});
	enumTableColumnInfos.append({false,	1,	tr("Column"),					String,		nullptr,	nullptr});
	enumTableColumnInfos.append({false,	2,	tr("Enumerator index"),			Integer,	nullptr,	nullptr});
	enumTableColumnInfos.append({false,	3,	tr("Enumerator name"),			String,		nullptr,	nullptr});
	enumTableColumnInfos.append({false,	4,	tr("Nested enumerator index"),	Integer,	nullptr,	nullptr});
	enumTableColumnInfos.append({false,	5,	tr("Nested enumerator name"),	String,		nullptr,	nullptr});
	
	writer->beginTable(tr("Enumerator names"), enumTableColumnInfos);
	emit callback_setProgressText(tr("Exporting enumerator names table..."));
	
	for (int tableIndex = 0; tableIndex < baseTables.size(); tableIndex++) {
		const Table* const baseTable = baseTables.at(tableIndex);
		const QList<ExportColumnInfo>& columnInfos = allColumnInfos.at(tableIndex);
		for (const ExportColumnInfo& columnInfo : columnInfos) {
			if (columnInfo.enumNames) {
				writer->writeEmptyRow(columnInfos);
				for (int i = 0; i < columnInfo.enumNames->size(); i++) {
					writer->beginRow();
					writer->writeCell(baseTable->uiName,			enumTableColumnInfos.at(0));
					writer->writeCell(columnInfo.name,				enumTableColumnInfos.at(1));
					writer->writeCell(i,							enumTableColumnInfos.at(2));
					writer->writeCell(columnInfo.enumNames->at(i),	enumTableColumnInfos.at(3));
					writer->writeCell(QVariant(),					enumTableColumnInfos.at(4));
					writer->writeCell(QVariant(),					enumTableColumnInfos.at(5));
					writer->endRow();
				}
			}
			
			if (columnInfo.enumNameLists) {
				for (int j = 0; j < columnInfo.enumNameLists->size(); j++) {
					writer->writeEmptyRow(columnInfos);
					const auto& [discerningEnumName, enumNames] = columnInfo.enumNameLists->at(j);
					for (int i = 0; i < enumNames.size(); i++) {
						writer->beginRow();
						writer->writeCell(baseTable->uiName,	enumTableColumnInfos.at(0));
						writer->writeCell(columnInfo.name,		enumTableColumnInfos.at(1));
						writer->writeCell(j,					enumTableColumnInfos.at(2));
						writer->writeCell(discerningEnumName,	enumTableColumnInfos.at(3));
						writer->writeCell(i,					enumTableColumnInfos.at(4));
						writer->writeCell(enumNames.at(i),		enumTableColumnInfos.at(5));
						writer->endRow();
					}
				}
			}
		}
	}
	
	writer->endTable();
}
