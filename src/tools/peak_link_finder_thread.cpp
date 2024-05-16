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
				QString searchString = peakName;
				if (peak->regionID.isValid()) {
					const ItemID rangeID = db.regionsTable.rangeIDColumn.getValueFor(FORCE_VALID(peak->regionID));
					if (rangeID.isValid()) {
						const QString rangeName = db.rangesTable.nameColumn.getValueFor(FORCE_VALID(rangeID)).toString();
						searchString += " " + rangeName;
					}
				}
				const QString sanitizedSearchString = PeakDialog::urlSanitize(searchString, "+");
				
				QUrl url("https://customsearch.googleapis.com/customsearch/v1");
				QUrlQuery query;
				query.addQueryItem("key", apiKey);
				query.addQueryItem("cx", "776b1f5ab722c4f75");
				query.addQueryItem("q", sanitizedSearchString);
				query.addQueryItem("num", "1");
				query.addQueryItem("safe", "active");
				query.addQueryItem("siteSearch", website);
				query.addQueryItem("siteSearchFilter", "i");
				url.setQuery(query);
				
				QNetworkAccessManager* const networkManager = new QNetworkAccessManager();
				
				bool waiting = true;
				QNetworkReply* reply = nullptr;
				connect(networkManager, &QNetworkAccessManager::finished, [&waiting, &reply](QNetworkReply* incomingReply) {
					reply = incomingReply;
					waiting = false;
				});
				
				// Send network request and wait for response
				networkManager->get(QNetworkRequest(url));
				while (waiting) {
					QCoreApplication::processEvents();
				}
				assert(reply);
				
				const QString request = reply->request().url().toString();
				const QString response = reply->readAll();
				const QString errorString = reply->errorString();
				reply->manager()->deleteLater();
				reply->deleteLater();
				
				// Parse JSON response
				const QJsonDocument json = QJsonDocument::fromJson(response.toUtf8());
				
				if (reply->error() != QNetworkReply::NoError) {
					// Parse and display error message
					const QString errorMessage = json["error"]["message"].toString();
					qDebug() << "Google query error:" << errorString << errorMessage;
					QMessageBox::critical(parent, tr("Google search error"), errorMessage);
					abort();
					continue;
				}
				
				// Check that there is at least one result
				if (json["items"].toArray().isEmpty()) {
					qDebug() << "No results for query" << request;
					continue;
				}
				
				// Get URL of first result
				wikiLink = json["items"].toArray()[0].toObject()["link"].toString();
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
