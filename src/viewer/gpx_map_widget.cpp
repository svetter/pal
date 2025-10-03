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

#include "gpx_map_widget.h"

#include "src/dialogs/ascent_dialog.h"
#include "src/settings/settings.h"

#include <QWebEngineSettings>



GpxMapWidget::GpxMapWidget(QWidget* parent) :
	QWidget(parent),
	db(nullptr),
	gpxFileServer(GpxFileServer()),
	currentAscentID(nullptr)
{
	setupUi(this);
	
	handle_filepathChanged();
	
	gpxFileServer.setup();
	
	// Allow http
	webEngineView->page()->settings()->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, true);
	
	connect(filepathLineEdit,	&QLineEdit::textChanged,		this,	&GpxMapWidget::handle_filepathChanged);
	connect(fileBrowseButton,	&QPushButton::clicked,			this,	&GpxMapWidget::handle_browseButtonClicked);
	connect(fileDropFrame,		&FileDropFrame::filesDropped,	this,	&GpxMapWidget::handle_filesDropped);
}

GpxMapWidget::~GpxMapWidget()
{}



void GpxMapWidget::supplyPointers(Database* const db, const ItemID* const currentAscentID)
{
	this->db = db;
	this->currentAscentID = currentAscentID;
}



void GpxMapWidget::ascentAboutToChange()
{
	if (currentAscentID && currentAscentID->isValid()) {
		saveFilepath();
	}
}

void GpxMapWidget::ascentChanged()
{
	const ItemID ascentID = *currentAscentID;
	if (ascentID.isInvalid()) {
		filepathLabel		->setEnabled(false);
		filepathLineEdit	->setEnabled(false);
		fileBrowseButton	->setEnabled(false);
		updateFileDropFrameProperties(true, false, false);
		return;
	}
	filepathLabel		->setEnabled(true);
	filepathLineEdit	->setEnabled(true);
	fileBrowseButton	->setEnabled(true);
	
	const QString newFilepath = db->ascentsTable.gpxFileColumn.getValueFor(FORCE_VALID(ascentID)).toString();
	filepathLineEdit->setText(newFilepath); // triggers handler
}



void GpxMapWidget::aboutToExit()
{
	saveFilepath();
}



void GpxMapWidget::handle_filepathChanged()
{
	if (!isVisible()) {
		updateFileDropFrameProperties(false, false, false);
		return;
	}
	
	const QString newFilepath = filepathLineEdit->text();
	
	if (newFilepath.isEmpty()) {
		updateFileDropFrameProperties(true, false, false);
		return;
	}
	
	const QString serverFilename = gpxFileServer.serveNewFile(newFilepath);
	if (serverFilename.isNull()) {
		updateFileDropFrameProperties(true, true, false);
		return;
	}
	
	const QString embedUrl = createGpxStudioEmbedUrl(serverFilename);
	webEngineView->setUrl(QUrl(embedUrl));
	
	updateFileDropFrameProperties(true, true, true);
}

void GpxMapWidget::handle_browseButtonClicked()
{
	// Determine path at which file dialog will open
	QString preSelectedDir = Settings::ascentDialog_preSelectedFilepathGpx.get();
	if (preSelectedDir.isEmpty()) preSelectedDir = QString(filepathLineEdit->text());
	if (preSelectedDir.isEmpty()) preSelectedDir = db->ascentsTable.gpxFileColumn.getValueFor(FORCE_VALID(*currentAscentID)).toString();
	
	const QString filepath = openFileDialogForGpxFileSelection(*this, preSelectedDir);
	if (filepath.isEmpty()) return;
	
	Settings::ascentDialog_preSelectedFilepathGpx.set(QFileInfo(filepath).path());
	filepathLineEdit->setText(filepath);
}

void GpxMapWidget::handle_filesDropped(QStringList filepaths)
{
	if (filepaths.isEmpty()) return;
	const QString filepath = filepaths.first();
	if (filepath.isEmpty()) return;
	
	Settings::ascentDialog_preSelectedFilepathGpx.set(QFileInfo(filepath).path());
	filepathLineEdit->setText(filepath);
}



void GpxMapWidget::updateFileDropFrameProperties(bool validAscent, bool fileSet, bool fileExists)
{
	assert(!fileSet || validAscent);
	assert(!fileExists || fileSet);
	
	if (!validAscent) {
		// Empty frame will be displayed
		fileDropFrame		->setFrameStyle(QFrame::StyledPanel);
		noFileGroupBox		->setVisible(false);
		fileErrorGroupBox	->setVisible(false);
		webEngineView		->setVisible(false);
		webEngineView		->setHtml("");
	}
	
	if (!fileSet) {
		// No map box will be displayed
		webEngineView		->setVisible(false);
		fileErrorGroupBox	->setVisible(false);
		noFileGroupBox		->setVisible(true);
		fileDropFrame		->setFrameStyle(QFrame::StyledPanel);
		fileDropFrame		->layout()->setContentsMargins(10, 10, 10, 10);
		webEngineView		->setHtml("");
		return;
	}
	
	else if (!fileExists) {
		// Map error box will be displayed
		webEngineView		->setVisible(false);
		noFileGroupBox		->setVisible(false);
		fileErrorGroupBox	->setVisible(true);
		fileDropFrame		->setFrameStyle(QFrame::StyledPanel);
		fileDropFrame		->layout()->setContentsMargins(10, 10, 10, 10);
		webEngineView		->setHtml("");
		return;
	}
	
	else {
		// Map will be displayed
		fileDropFrame		->setFrameStyle(QFrame::NoFrame);
		fileDropFrame		->layout()->setContentsMargins(0, 0, 0, 0);
		noFileGroupBox		->setVisible(false);
		fileErrorGroupBox	->setVisible(false);
		webEngineView		->setVisible(true);
	}
}

QString GpxMapWidget::createGpxStudioEmbedUrl(const QString& serverFilename)
{
	const QString serverIp = gpxFileServer.ip;
	const QString ipWithBrackets = serverIp.contains(":") ? QString("[%1]").arg(serverIp) : serverIp;
	const QString port = QString::number(gpxFileServer.port);
	const QString gpxFileUrl = QString("http://%1:%2/files/%3").arg(ipWithBrackets, port, serverFilename);
	
	const QString mapboxToken = Settings::mapboxToken.get();
	const QString mapType = Settings::ascentViewer_defaultMapType.get();
	const QString showElevationProfile = QVariant(Settings::ascentViewer_showElevationProfile.get()).toString();
	const QString theme = Settings::uiColorScheme.get();
	
	const QString optionsJson = QString(R"({
		"token": "%1",
		"files": ["%2"],
		"basemap": "%3",
		"elevation": {
			"show": %4,
			"height": "175"
		},
		"distanceMarkers": true,
		"directionMarkers": true,
		"distanceUnits": "metric",
		"velocityUnits": "speed",
		"temperatureUnits": "celsius",
		"theme": "%5"
	})").arg(
		mapboxToken,
		gpxFileUrl,
		mapType,
		showElevationProfile,
		theme
	);
	
	const QString optionsEncoded = QUrl::toPercentEncoding(optionsJson);
	const QString embedUrl = QString("https://gpx.studio/embed?options=%1").arg(optionsEncoded);
	return embedUrl;
}

void GpxMapWidget::saveFilepath()
{
	const ItemID ascentID = *currentAscentID;
	if (ascentID.isInvalid()) return;
	const QString savedFilepath = db->ascentsTable.gpxFileColumn.getValueFor(FORCE_VALID(ascentID)).toString();
	const QString currentFilepath = filepathLineEdit->text();
	if (savedFilepath == currentFilepath) return;
	
	db->beginChangingData();
	db->ascentsTable.updateCell(*this, FORCE_VALID(ascentID), db->ascentsTable.gpxFileColumn, currentFilepath);
	db->finishChangingData();
}



void GpxMapWidget::showEvent(QShowEvent* event)
{
	Q_UNUSED(event);
	handle_filepathChanged();
}
