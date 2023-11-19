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
 * @file export_dialog.cpp
 * 
 * This file defines the DataExportDialog class.
 */

#include "export_dialog.h"

#include <QMessageBox>
#include <QFileDialog>



/**
 * Creates a new DataExportDialog.
 * 
 * Prepares and connects the UI.
 * 
 * @param parent		The parent window.
 * @param typesHandler	The project item types handler.
 */
DataExportDialog::DataExportDialog(QWidget* parent, const ItemTypesHandler* typesHandler) :
		QDialog(parent),
		typesHandler(typesHandler),
		running(false),
		aborted(false),
		extensions(QMap<ExportFormat, QString>()),
		csvSeparators(QList<QString>())
{
	setupUi(this);
	
	extensions.insert(CSV, "csv");
	extensions.insert(FODS, "fods");
	
	csvSeparators.append(",");
	fileFormatCsvSeparatorComboBox->insertItem(0, tr("Comma"));
	csvSeparators.append("\t");
	fileFormatCsvSeparatorComboBox->insertItem(1, tr("Tab"));
	
	updateEnableUI();
	
	
	connect(bottomButtonBox->button(QDialogButtonBox::Close),	&QPushButton::clicked,	this,	&DataExportDialog::handle_close);

	connect(exportModeOneTableRadio,	&QRadioButton::clicked,		this,	&DataExportDialog::updateEnableUI);
	connect(exportModeAsShownRadio,		&QRadioButton::clicked,		this,	&DataExportDialog::updateEnableUI);
	connect(exportModeRawRadio,			&QRadioButton::clicked,		this,	&DataExportDialog::updateEnableUI);
	
	connect(filepathBrowseButton,		&QPushButton::clicked,		this,	&DataExportDialog::handle_browseFilepath);
	connect(filepathLineEdit,			&QLineEdit::textChanged,	this,	&DataExportDialog::handle_filepathChanged);
	
	connect(fileFormatCsvRadio,			&QRadioButton::clicked,		this,	&DataExportDialog::handle_fileFormatChanged);
	connect(fileFormatFodsRadio,		&QRadioButton::clicked,		this,	&DataExportDialog::handle_fileFormatChanged);
	
	connect(startButton,				&QPushButton::clicked,		this,	&DataExportDialog::handle_start);
	connect(abortButton,				&QPushButton::clicked,		this,	&DataExportDialog::handle_abort);
}



/**
 * Event handler for the filepath line edit.
 * 
 * Sets the file format according to the extension, if possible, then updates the UI.
 */
void DataExportDialog::handle_filepathChanged()
{
	setFileFormatFromExtension(filepathLineEdit->text());
	updateEnableUI();
}

/**
 * Event handler for the "browse" button.
 * 
 * Opens a file dialog to select the destination filepath and sets the path in the line edit if
 * valid.
 */
void DataExportDialog::handle_browseFilepath()
{
	QString caption = tr("Select save location");
	QString preSelectedFilepath = filepathLineEdit->text();
	if (preSelectedFilepath.isEmpty()) preSelectedFilepath = QFileInfo(Settings::lastOpenDatabaseFile.get()).absolutePath();
	if (preSelectedFilepath.isEmpty()) preSelectedFilepath = QDir::homePath();
	
	QString filter = DataExportDialog::tr("Spreadsheets") + " (*.csv *.fods *.ods *.xlsx *.xlsm *.xls *.gsheet);;"
					+ DataExportDialog::tr("All files") + " (*)";
	QString filepath = QFileDialog::getSaveFileName(this, caption, preSelectedFilepath, filter, nullptr, QFileDialog::Options());
	setFileFormatFromExtension(filepath);
	filepath = enforceExtension(filepath);
	
	if (!filepath.isEmpty()) filepathLineEdit->setText(filepath);
}

/**
 * Event handler for the file format radio buttons.
 * 
 * Updates the file extension in the filepath line edit and updates the UI.
 */
void DataExportDialog::handle_fileFormatChanged()
{
	filepathLineEdit->setText(enforceExtension(filepathLineEdit->text()));
	updateEnableUI();
}



/**
 * Event handler for the "start" button.
 * 
 * Starts the export process.
 */
void DataExportDialog::handle_start()
{
	assert(!running);
	
	running = true;
	aborted = false;
	updateEnableUI();
	
	progressBar->setMinimum(0);
	progressBar->setValue(0);
	
	// Collect parameters
	ExportMode mode				= getCurrentlySelectedExportMode();
	bool includeStats			= exportModeAsShownStatsCheckbox->isChecked();
	ExportFormat fileFormat		= getCurrentlySelectedFileFormat();
	const QString& csvSeparator	= getCurrentlySelectedCsvSeparator();
	
	workerThread = new DataExportThread(this, typesHandler, mode, includeStats, filepathLineEdit->text(), fileFormat, csvSeparator);
	
	connect(workerThread, &DataExportThread::callback_reportWorkloadSize,	this,	&DataExportDialog::handle_callback_workloadSize);
	connect(workerThread, &DataExportThread::callback_reportProgress,		this,	&DataExportDialog::handle_callback_progressUpdate);
	connect(workerThread, &DataExportThread::callback_setProgressText,		this,	&DataExportDialog::handle_callback_progressTextUpdate);
	connect(workerThread, &DataExportThread::finished,						this,	&DataExportDialog::handle_finished);
	
	workerThread->start();
}

/**
 * Event handler for the worker thread's "finished" signal.
 * 
 * Cleans up the worker thread and updates the UI.
 * Note that this is also called if the thread was aborted.
 */
void DataExportDialog::handle_finished()
{
	assert(running);
	
	if (aborted) {
		progressLabel->setText(tr("Export aborted."));
	} else {
		progressLabel->setText(tr("Export finished successfully."));
	}
	
	workerThread->wait();
	workerThread->deleteLater();
	workerThread = nullptr;
	
	running = false;
	updateEnableUI(false);
}

/**
 * Event handler for the "abort" button.
 * 
 * Aborts the export process.
 */
void DataExportDialog::handle_abort()
{
	assert(running);
	
	aborted = true;
	workerThread->abort();
}

/**
 * Event handler for the "close" button.
 * 
 * Closes the dialog.
 * If the relocation process is running, asks the user if the process should be aborted.
 */
void DataExportDialog::handle_close()
{
	if (!running) return accept();
	
	QMessageBox::StandardButton resultButton;
	
	QString title = tr("Operation running");
	QString question = tr("Do you want to abort the running process?");
	auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
	auto selected = QMessageBox::Cancel;
	resultButton = QMessageBox::question(this, title, question, options, selected);
	
	if (resultButton == QMessageBox::Yes) {
		handle_abort();
		accept();
	}
}



/**
 * Callback function for the worker thread reporting back its workload size after starting.
 * 
 * @param workloadSize	The number of cells which need to be updated and exported (additive).
 */
void DataExportDialog::handle_callback_workloadSize(int workloadSize)
{
	progressBar->setMaximum(workloadSize);
}

/**
 * Callback function for the worker thread reporting back its progress.
 * 
 * @param processed	The number of cells updated or exported (additive) so far.
 */
void DataExportDialog::handle_callback_progressUpdate(int processed)
{
	progressBar->setValue(processed);
}

/**
 * Callback function for the worker thread reporting back a string for the current work chunk.
 * 
 * @param progressString	The string to display next to the progress bar.
 */
void DataExportDialog::handle_callback_progressTextUpdate(const QString& progressString)
{
	progressLabel->setText(progressString);
}



/**
 * Updates the enabled state of the UI elements.
 * 
 * @param resetProgress	Whether to reset the progress bar and label.
 */
void DataExportDialog::updateEnableUI(bool resetProgress)
{
	// Options
	exportModeAsShownLabel			->setEnabled(exportModeAsShownRadio	->isChecked());
	exportModeAsShownStatsCheckbox	->setEnabled(exportModeAsShownRadio	->isChecked());
	exportModeRawLabel				->setEnabled(exportModeRawRadio		->isChecked());
	fileFormatCsvSeparatorLabel		->setEnabled(fileFormatCsvRadio		->isChecked());
	fileFormatCsvSeparatorComboBox	->setEnabled(fileFormatCsvRadio		->isChecked());
	fileFormatFodsLabel				->setEnabled(fileFormatFodsRadio	->isChecked());
	
	exportModeGroupBox	->setEnabled(!running);
	outputFilesGroupBox	->setEnabled(!running);
	
	// Start and abort
	bool canStart = !filepathLineEdit->text().isEmpty()
			&& getCurrentlySelectedExportMode() != ExportMode(-1)
			&& getCurrentlySelectedFileFormat() != ExportFormat(-1);
	startButton->setEnabled(!running && canStart);
	abortButton->setEnabled(running);
	
	// Progress bar & label
	if (resetProgress) {
		progressBar->reset();
		progressLabel->clear();
	}
}


/**
 * Returns the currently selected export mode.
 * 
 * @return	The currently selected export mode.
 */
ExportMode DataExportDialog::getCurrentlySelectedExportMode() const
{
	if (exportModeOneTableRadio->isChecked()) {
		return OneTable;
	} else 	if (exportModeAsShownRadio->isChecked()) {
		return AsShown;
	} else if (exportModeRawRadio->isChecked()) {
		return Raw;
	}
	return ExportMode(-1);
}

/**
 * Returns the currently selected file format.
 * 
 * @return	The currently selected file format.
 */
ExportFormat DataExportDialog::getCurrentlySelectedFileFormat() const
{
	if (fileFormatCsvRadio->isChecked()) {
		return CSV;
	} else if (fileFormatFodsRadio->isChecked()) {
		return FODS;
	}
	return ExportFormat(-1);
}

/**
 * Returns the file extension corresponding to the currently selected file format.
 * 
 * @return	The file extension for the currently selected file format.
 */
QString DataExportDialog::getCurrentlySelectedFileFormatExtension() const
{
	return extensions[getCurrentlySelectedFileFormat()];
}

/**
 * Returns the CSV separator string fot the currently selected option from the combo box.
 * 
 * @return	The CSV separator string currently selected in the combo box.
 */
const QString& DataExportDialog::getCurrentlySelectedCsvSeparator() const
{
	return csvSeparators.at(fileFormatCsvSeparatorComboBox->currentIndex());
}


/**
 * Ensures that the path in the filepath line edit has the correct extension.
 * 
 * Appends the extension for the currently selected file format if the path has no extension or an
 * unknown extension. Does nothing if the path is empty or already has the correct extension.
 * 
 * @param filepath	The filepath to enforce the extension on.
 * @return			The filepath with the correct extension, or an empty string.
 */
QString DataExportDialog::enforceExtension(const QString& filepath) const
{
	if (filepath.isNull() || filepath.isEmpty()) return filepath;
	
	QString newExtension = getCurrentlySelectedFileFormatExtension();
	
	QFileInfo fileInfo = QFileInfo(filepath);
	if (fileInfo.suffix().isEmpty()) {
		return filepath + "." + newExtension;
	}
	
	if (QString::compare(fileInfo.suffix(), newExtension, Qt::CaseInsensitive) == 0) {
		return filepath;
	}
	
	bool extensionMatchesAnyKnown = false;
	for (const QString& extension : extensions) {
		if (QString::compare(fileInfo.suffix(), extension, Qt::CaseInsensitive)) {
			extensionMatchesAnyKnown = true;
			break;
		}
	}
	if (extensionMatchesAnyKnown) {
		return filepath.chopped(fileInfo.suffix().size()) + newExtension;
	} else {
		return filepath + "." + newExtension;
	}
}

/**
 * Sets the file format radio button according to the extension of the given filepath.
 * 
 * If the filepath has no extension or an unknown extension, does nothing.
 * 
 * @param filepath	The filepath to get the extension from.
 */
void DataExportDialog::setFileFormatFromExtension(const QString& filepath) const
{
	QString extension = QFileInfo(filepath).suffix();
	if (extension.isEmpty()) {
		return;
	}
	
	for (auto iter = extensions.constBegin(); iter != extensions.constEnd(); iter++) {
		if (QString::compare(iter.value(), extension, Qt::CaseInsensitive) == 0) {
			switch (iter.key()) {
			case CSV: {
				fileFormatCsvRadio->setChecked(true);
				break;
			}
			case FODS: {
				fileFormatFodsRadio->setChecked(true);
				break;
			}
			default: assert(false);
			}
		}
	}
}



/**
 * Redirects the "close" event to handle_close().
 */
void DataExportDialog::reject()
{
	handle_close();
}
