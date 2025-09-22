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
	ip(getPublicIp())
{
	gpxFilePaths += "D:\\Temp\\GPX\\simple.gpx";
	gpxFilePaths += "D:\\Temp\\GPX\\gpx1.gpx";
	gpxFilePaths += "D:\\Temp\\GPX\\gpx2.gpx";
}

void GpxFileServer::startServer()
{
	// Add route with filename parameter
	server.route("/files/<arg>", [=] (const QString& filename) {
		QString filePath;
		
		// Match only exact whitelisted filenames
		for (const QString &f : std::as_const(gpxFilePaths)) {
			QFileInfo fi(f);
			if (fi.fileName() == filename) {
				filePath = fi.absoluteFilePath();
				break;
			}
		}
		
		if (filePath.isEmpty()) {
			return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
		}
		
		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly)) {
			return QHttpServerResponse(QHttpServerResponder::StatusCode::InternalServerError);
		}
		
		QByteArray data = file.readAll();
		file.close();
		
		QHttpServerResponse response = QHttpServerResponse(data);
		setHttpHeadersFor(response);
		return response;
	});
	
	// Handle preflight OPTIONS requests for CORS
	server.route("/files/<arg>", QHttpServerRequest::Method::Options, [](const QString &) {
		QHttpServerResponse response = QHttpServerResponse(QHttpServerResponse::StatusCode::Ok);
		setHttpHeadersFor(response);
		return response;
	});
	
	// Listen on an available port
	tcpServer = new QTcpServer();
	if (!tcpServer->listen(QHostAddress::Any, port) || !server.bind(tcpServer)) {
		qDebug() << "HTTP server: Listening failed";
		delete tcpServer;
		return;
	}
	qDebug() << "HTTP server: Listening on port" << port;
	
	for (const QString& file : std::as_const(gpxFilePaths)) {
		const QString filename = QFileInfo(file).fileName();
		qDebug() << QString("Serving: %1 -> http://%2:%3/files/%4").arg(filename, ip).arg(port).arg(filename);
	}
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
	QNetworkReply* const reply = networkAccessManager.get(request);
	
	QEventLoop loop;
	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();
	
	const QByteArray response = reply->readAll();
	reply->deleteLater();
	
	return response;
}
