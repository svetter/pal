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
 * @file relocate_photos_dialog.cpp
 * 
 * This file defines the RelocatePhotosDialog class.
 */

#include "relocate_photos_dialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>



/**
 * Creates a new RelocatePhotosDialog.
 * 
 * Prepares and connects the UI.
 * 
 * @param parent	The parent window.
 * @param db		The project database.
 */
RelocatePhotosDialog::RelocatePhotosDialog(QWidget* parent, Database* db) :
		QDialog(parent),
		db(db),
		running(false)
{
	setupUi(this);
	
	updateEnableUI();
	
	
	connect(bottomButtonBox->button(QDialogButtonBox::Close),	&QPushButton::clicked,	this,	&RelocatePhotosDialog::handle_close);
	
	connect(oldPathBrowseButton,	&QPushButton::clicked,		this,	&RelocatePhotosDialog::handle_browseOldPath);
	connect(newPathBrowseButton,	&QPushButton::clicked,		this,	&RelocatePhotosDialog::handle_browseNewPath);
	
	connect(oldPathLineEdit,		&QLineEdit::textChanged,	this,	&RelocatePhotosDialog::updateEnableUI);
	connect(newPathLineEdit,		&QLineEdit::textChanged,	this,	&RelocatePhotosDialog::updateEnableUI);
	
	connect(startButton,			&QPushButton::clicked,		this,	&RelocatePhotosDialog::handle_start);
	connect(abortButton,			&QPushButton::clicked,		this,	&RelocatePhotosDialog::handle_abort);
}



/**
 * Event handler for the "browse old path" button.
 * 
 * Opens a file dialog to select the old path and sets the path in the line edit if valid.
 */
void RelocatePhotosDialog::handle_browseOldPath()
{
	QString caption = tr("Select old location of photos");
	QString preSelectedDir = oldPathLineEdit->text();
	if (preSelectedDir.isEmpty()) preSelectedDir = newPathLineEdit->text();
	if (preSelectedDir.isEmpty()) preSelectedDir = QDir::homePath();
	QString oldPath = QFileDialog::getExistingDirectory(this, caption, preSelectedDir);
	if (!oldPath.isEmpty()) oldPathLineEdit->setText(oldPath);
}

/**
 * Event handler for the "browse new path" button.
 * 
 * Opens a file dialog to select the new path and sets the path in the line edit if valid.
 */
void RelocatePhotosDialog::handle_browseNewPath()
{
	QString caption = tr("Select new location of photos");
	QString preSelectedDir = newPathLineEdit->text();
	if (preSelectedDir.isEmpty()) preSelectedDir = oldPathLineEdit->text();
	if (preSelectedDir.isEmpty()) preSelectedDir = QDir::homePath();
	QString newPath = QFileDialog::getExistingDirectory(this, caption, preSelectedDir);
	if (!newPath.isEmpty()) oldPathLineEdit->setText(newPath);
}



/**
 * Event handler for the "start" button.
 * 
 * Starts the relocation process.
 */
void RelocatePhotosDialog::handle_start()
{
	assert(!running);
	
	running = true;
	updateEnableUI();
	
	progressBar->setMinimum(0);
	progressBar->setValue(0);
	
	workerThread = new PhotoRelocationThread(this, db, oldPathLineEdit->text(), newPathLineEdit->text());
	connect(workerThread, &PhotoRelocationThread::callback_reportWorkloadSize,	this,	&RelocatePhotosDialog::handle_callback_workloadSize);
	connect(workerThread, &PhotoRelocationThread::callback_reportProgress,		this,	&RelocatePhotosDialog::handle_callback_progressUpdate);
	connect(workerThread, &PhotoRelocationThread::callback_updateFilepathAt,	this,	&RelocatePhotosDialog::handle_callback_updateFilepath);
	connect(workerThread, &PhotoRelocationThread::finished,						this,	&RelocatePhotosDialog::handle_finished);
	
	workerThread->start();
}

/**
 * Event handler for the worker thread's "finished" signal.
 * 
 * Cleans up the worker thread and updates the UI.
 * Note that this is also called if the thread was aborted.
 */
void RelocatePhotosDialog::handle_finished()
{
	assert(running);
	
	workerThread->wait();
	workerThread->deleteLater();
	workerThread = nullptr;
	
	running = false;
	updateEnableUI();
}

/**
 * Event handler for the "abort" button.
 * 
 * Aborts the relocation process.
 */
void RelocatePhotosDialog::handle_abort()
{
	assert(running);
	
	workerThread->abort();
}

/**
 * Event handler for the "close" button.
 * 
 * Closes the dialog.
 * If the relocation process is running, asks the user if the process should be aborted.
 */
void RelocatePhotosDialog::handle_close()
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
 * @param workloadSize	The number of photos found that need to be relocated.
 */
void RelocatePhotosDialog::handle_callback_workloadSize(int workloadSize)
{
	progressBar->setMaximum(workloadSize);
}

/**
 * Callback function for the worker thread reporting back its progress.
 * 
 * @param processed	The number of photos processed so far.
 * @param updated	The number of photos whose location has been updated so far.
 */
void RelocatePhotosDialog::handle_callback_progressUpdate(int processed, int updated)
{
	progressBar->setValue(processed);
	feedbackLabel->setText(tr("Photo locations updated: %1").arg(updated));
}

/**
 * Callback function for the worker thread delegating a photo location update to the database.
 * 
 * @param bufferRowIndex	The index of the photo in the photos table buffer.
 * @param newFilepath		The new filepath of the photo.
 */
void RelocatePhotosDialog::handle_callback_updateFilepath(BufferRowIndex bufferRowIndex, QString newFilepath)
{
	return db->photosTable->updateFilepathAt(this, bufferRowIndex, newFilepath);
}



/**
 * Updates the enabled state of the UI elements.
 */
void RelocatePhotosDialog::updateEnableUI()
{
	oldPathLineEdit->setEnabled(!running);
	newPathLineEdit->setEnabled(!running);
	bool canStart = !oldPathLineEdit->text().isEmpty() || !newPathLineEdit->text().isEmpty();
	startButton->setEnabled(!running && canStart);
	abortButton->setEnabled(running);
}



/**
 * Redirects the "close" event to handle_close().
 */
void RelocatePhotosDialog::reject()
{
	handle_close();
}
