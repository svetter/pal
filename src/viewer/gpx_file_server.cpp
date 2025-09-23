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

#include "gpx_file_server.h"

#include <QFileInfo>
#include <QNetworkInterface>
#include <QEventLoop>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>



GpxFileServer::GpxFileServer() :
	server(QHttpServer()),
	tcpServer(nullptr),
	currentFile(nullptr),
	currentFileIndex(0),
	ip(getPublicIp())
{}

GpxFileServer::~GpxFileServer()
{
	delete tcpServer;
}



void GpxFileServer::setup()
{
	// Add route with filename parameter
	// Handle file requests
	server.route("/files/<arg>", [=] (const QString& filename) {
		if (filename.isEmpty() || !currentFile) {
			return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
		}
		
		if (filename != getCurrentServersideFilename()) {
			return QHttpServerResponse(QHttpServerResponder::StatusCode::Forbidden);
		}
		
		if (!currentFile->open(QIODevice::ReadOnly)) {
			return QHttpServerResponse(QHttpServerResponder::StatusCode::InternalServerError);
		}
		
		const QByteArray data = currentFile->readAll();
		currentFile->close();
		
		QHttpServerResponse response = QHttpServerResponse(data);
		setHttpHeadersFor(response);
		return response;
	});
	
	// Handle OPTIONS requests
	server.route("/files/<arg>", QHttpServerRequest::Method::Options, [] (const QString&) {
		QHttpServerResponse response = QHttpServerResponse(QHttpServerResponse::StatusCode::Ok);
		setHttpHeadersFor(response);
		return response;
	});
	
	// Listen
	delete tcpServer;
	tcpServer = new QTcpServer();
	if (!tcpServer->listen(QHostAddress::Any, port) || !server.bind(tcpServer)) {
		qDebug() << "GPX HTTP server: Listening failed";
		delete tcpServer;
		return;
	}
}

QString GpxFileServer::serveNewFile(const QString& filepath)
{
	delete currentFile;
	currentFile = new QFile(filepath);
	currentFileIndex++;
	
	if (!currentFile->exists()) {
		return QString();
	}
	return getCurrentServersideFilename();
}



QString GpxFileServer::getCurrentServersideFilename() const
{
	return QString::number(currentFileIndex) + ".gpx";
}

void GpxFileServer::setHttpHeadersFor(QHttpServerResponse& serverResponse)
{
	QHttpHeaders headers = QHttpHeaders();
	headers.append(QHttpHeaders::WellKnownHeader::AccessControlAllowOrigin,		"https://gpx.studio");
	headers.append(QHttpHeaders::WellKnownHeader::AccessControlAllowMethods,	"GET, OPTIONS");
	headers.append(QHttpHeaders::WellKnownHeader::AccessControlAllowHeaders,	"Content-Type");
	serverResponse.setHeaders(headers);
}

QString GpxFileServer::getPublicIp()
{
	const QNetworkRequest request(QUrl("https://api64.ipify.org"));
	QNetworkAccessManager networkAccessManager = QNetworkAccessManager();
	QNetworkReply* const reply = networkAccessManager.get(request);
	
	QEventLoop loop = QEventLoop();
	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();
	
	const QByteArray response = reply->readAll();
	reply->deleteLater();
	
	return response;
}
