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
 * @file peak_link_finder_thread.cpp
 * 
 * This file defines the PeakLinkFinderThread class.
 */

#include "peak_link_finder_thread.h"

#include "src/settings/settings.h"
#include "src/dialogs/peak_dialog.h"

#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>



/**
 * Creates a new PeakLinkFinderThread.
 * 
 * @param parent			The parent window of this thread.
 * @param db				The database to work on.
 * @param maps				Whether to update Google Maps links.
 * @param earth				Whether to update Google Earth links.
 * @param wiki				Whether to update Wikipedia links.
 * @param replaceExisting	Whether to replace existing links.
 */
PeakLinkFinderThread::PeakLinkFinderThread(QDialog* parent, Database& db, bool maps, bool earth, bool wiki, bool replaceExisting) :
	QThread(parent),
	parent(parent),
	db(db),
	apiKey(Settings::googleApiKey.get()),
	maps(maps),
	earth(earth),
	wiki(wiki),
	replaceExisting(replaceExisting),
	workloadSize(-1),
	abortWasCalled(false)
{}



/**
 * Starts the thread.
 * 
 * Determines workload size and reports it back via the callback signal reportWorkloadSize().
 * Then iterates over all peaks and searches for the requested links, delegating the actual updates
 * of the database to the callback signal callback_updateFilepathAt().
 * 
 * After each iteration, the progress is reported back via the callback signal reportProgress().
 * 
 * If abort() is called while the thread is running, the thread will stop after completing the
 * current iteration.
 */
void PeakLinkFinderThread::run()
{
	workloadSize = db.peaksTable.getNumberOfRows();
	emit callback_reportWorkloadSize(workloadSize);
	
	for (BufferRowIndex index = BufferRowIndex(0); index.isValid(workloadSize); index++) {
		if (abortWasCalled) break;
		
		unique_ptr<Peak> peak = db.getPeakAt(index);
		const QString peakName = peak->name;
		
		if (PeakDialog::urlSanitize(peakName, "").isEmpty()) {
			emit callback_reportProgress(index.get() + 1);
			continue;
		}
		
		QString mapsLink = QString();
		if (maps && (peak->mapsLink.isEmpty() || replaceExisting)) {
			const QString sanitizedPeakName = PeakDialog::urlSanitize(peakName, "+");
			mapsLink = "https://www.google.com/maps/search/" + sanitizedPeakName;
		}
		
		QString earthLink = QString();
		if (earth && (peak->earthLink.isEmpty() || replaceExisting)) {
			const QString sanitizedPeakName = PeakDialog::urlSanitize(peakName, "+");
			earthLink = "https://earth.google.com/web/search/" + sanitizedPeakName;
		}
		
		QString wikiLink = QString();
		if (wiki && (peak->wikiLink.isEmpty() || replaceExisting)) {
			const QString website = tr("en") + ".wikipedia.org";
			
			if (apiKey.isEmpty()) {
				const QString sanitizedPeakName = PeakDialog::urlSanitize(peakName, "_");
				wikiLink = "https://" + website + "/wiki/" + sanitizedPeakName;
			}
			else {
				wikiLink = searchForLink(*peak, website);
				if (abortWasCalled) break;
			}
		}
		
		if (!mapsLink.isEmpty() || !earthLink.isEmpty() || !wikiLink.isEmpty()) {
			emit callback_updateLinksAt(index, mapsLink, earthLink, wikiLink);
		}
		emit callback_reportProgress(index.get() + 1);
	}
}

/**
 * Gracefully aborts the thread.
 */
void PeakLinkFinderThread::abort() {
	abortWasCalled = true;
}



/**
 * Searches for a link to the given peak on the given website.
 * 
 * Triggers an abort if an error occurs during the search.
 * 
 * @param peak		The peak to search for.
 * @param website	The website to search on.
 * @return			The found link, or an empty string if no link was found.
 */
QString PeakLinkFinderThread::searchForLink(const Peak& peak, const QString website)
{
	const QUrl url = PeakDialog::createLinkSearchUrl(db, website, peak.name, peak.regionID);
	
	QNetworkAccessManager* const networkManager = new QNetworkAccessManager();
	
	bool waiting = true;
	QNetworkReply* reply = nullptr;
	auto runWhenFinished = [&waiting, &reply](QNetworkReply* incomingReply) {
		reply = incomingReply;
		waiting = false;
	};
	connect(networkManager, &QNetworkAccessManager::finished, runWhenFinished);
	
	// Send network request and wait for response
	networkManager->get(QNetworkRequest(url));
	while (waiting) {
		QCoreApplication::processEvents();
	}
	assert(reply);
	
	const QPair<bool, QString> resultPair = PeakDialog::parseLinkSearchResponse(reply);
	const bool success = resultPair.first;
	const QString resultString = resultPair.second;
	
	if (!success) {
		QMessageBox::critical(parent, tr("Google search error"), resultString);
		abort();
		return QString();
	}
	
	if (resultString.isEmpty()) {
		return QString();
	}
	
	return resultString;
}
