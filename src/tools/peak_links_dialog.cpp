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
 * @file peak_links_dialog.cpp
 * 
 * This file defines the PeakLinksDialog class.
 */

#include "peak_links_dialog.h"

#include "src/settings/settings.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>



/**
 * Creates a new PeakLinksDialog.
 * 
 * Prepares and connects the UI.
 * 
 * @param parent	The parent window.
 * @param db		The project database.
 */
PeakLinksDialog::PeakLinksDialog(QWidget& parent, Database& db) :
	QDialog(&parent),
	db(db),
	running(false)
{
	setupUi(this);
	
	apiKeyHintLabel->setVisible(Settings::googleApiKey.get().isEmpty());
	updateEnableUI();
	
	
	connect(bottomButtonBox->button(QDialogButtonBox::Close),	&QPushButton::clicked,	this,	&PeakLinksDialog::handle_close);
	
	connect(mapsCheckbox,	&QCheckBox::toggled,	this,	&PeakLinksDialog::updateEnableUI);
	connect(earthCheckbox,	&QCheckBox::toggled,	this,	&PeakLinksDialog::updateEnableUI);
	connect(wikiCheckbox,	&QCheckBox::toggled,	this,	&PeakLinksDialog::updateEnableUI);
	
	connect(startButton,	&QPushButton::clicked,	this,	&PeakLinksDialog::handle_start);
	connect(stopButton,		&QPushButton::clicked,	this,	&PeakLinksDialog::handle_abort);
}



/**
 * Event handler for the "start" button.
 * 
 * Starts the link finding process.
 */
void PeakLinksDialog::handle_start()
{
	assert(!running);
	
	running = true;
	updateEnableUI();
	
	progressBar->setMinimum(0);
	progressBar->setValue(0);
	
	const bool maps				= mapsCheckbox->isChecked();
	const bool earth			= earthCheckbox->isChecked();
	const bool wiki				= wikiCheckbox->isChecked();
	const bool replaceExisting	= replaceExistingLinksCheckbox->isChecked();
	workerThread = new PeakLinkFinderThread(this, db, maps, earth, wiki, replaceExisting);
	connect(workerThread, &PeakLinkFinderThread::callback_reportWorkloadSize,	this,	&PeakLinksDialog::handle_callback_workloadSize);
	connect(workerThread, &PeakLinkFinderThread::callback_reportProgress,		this,	&PeakLinksDialog::handle_callback_progressUpdate);
	connect(workerThread, &PeakLinkFinderThread::callback_updateLinksAt,		this,	&PeakLinksDialog::handle_callback_updateLinksAt);
	connect(workerThread, &PeakLinkFinderThread::finished,						this,	&PeakLinksDialog::handle_finished);
	
	db.beginChangingData();
	workerThread->start();
}

/**
 * Event handler for the worker thread's "finished" signal.
 * 
 * Cleans up the worker thread and updates the UI.
 * Note that this is also called if the thread was aborted.
 */
void PeakLinksDialog::handle_finished()
{
	assert(running);
	
	workerThread->wait();
	workerThread->deleteLater();
	workerThread = nullptr;
	
	running = false;
	updateEnableUI();
	
	db.finishChangingData();
}

/**
 * Event handler for the "stop" button.
 * 
 * Aborts the link finding process.
 */
void PeakLinksDialog::handle_abort()
{
	assert(running);
	
	workerThread->abort();
}

/**
 * Event handler for the "close" button.
 * 
 * Closes the dialog.
 * If the link finding process is running, asks the user if the process should be aborted.
 */
void PeakLinksDialog::handle_close()
{
	if (!running) return accept();
	
	QMessageBox::StandardButton resultButton;
	
	QString title = tr("Operation running");
	QString question = tr("Do you want to stop the running process?");
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
 * @param workloadSize	The number of peaks.
 */
void PeakLinksDialog::handle_callback_workloadSize(int workloadSize)
{
	progressBar->setMaximum(workloadSize);
}

/**
 * Callback function for the worker thread reporting back its progress.
 * 
 * @param processed	The number of peaks processed so far.
 */
void PeakLinksDialog::handle_callback_progressUpdate(int processed)
{
	progressBar->setValue(processed);
}

/**
 * Callback function for the worker thread delegating a photo location update to the database.
 * 
 * @param bufferRowIndex	The index of the photo in the peaks table buffer.
 * @param mapsLink			The new Google Maps link for the photo. An empty string if not to be updated.
 * @param earthLink			The new Google Earth link for the photo. An empty string if not to be updated.
 * @param wikiLink			The new Wikipedia link for the photo. An empty string if not to be updated.
 */
void PeakLinksDialog::handle_callback_updateLinksAt(BufferRowIndex bufferRowIndex, QString mapsLink, QString earthLink, QString wikiLink)
{
	const ValidItemID peakID = VALID_ITEM_ID(db.peaksTable.primaryKeyColumn.getValueAt(bufferRowIndex));
	
	if (!mapsLink.isEmpty())	db.peaksTable.updateCell(*this, peakID, db.peaksTable.mapsLinkColumn,	mapsLink);
	if (!earthLink.isEmpty())	db.peaksTable.updateCell(*this, peakID, db.peaksTable.earthLinkColumn,	earthLink);
	if (!wikiLink.isEmpty())	db.peaksTable.updateCell(*this, peakID, db.peaksTable.wikiLinkColumn,	wikiLink);
}



/**
 * Updates the enabled state of the UI elements.
 */
void PeakLinksDialog::updateEnableUI()
{
	mapsCheckbox->setEnabled(!running);
	earthCheckbox->setEnabled(!running);
	wikiCheckbox->setEnabled(!running);
	replaceExistingLinksCheckbox->setEnabled(!running);
	apiKeyHintLabel->setEnabled(wikiCheckbox->isChecked());
	const bool canStart = mapsCheckbox->isChecked() || earthCheckbox->isChecked() || wikiCheckbox->isChecked();
	startButton->setEnabled(!running && canStart);
	stopButton->setEnabled(running);
}



/**
 * Redirects the "close" event to handle_close().
 */
void PeakLinksDialog::reject()
{
	handle_close();
}
