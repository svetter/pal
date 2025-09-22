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

#ifndef GPX_FILE_SERVER_H
#define GPX_FILE_SERVER_H

#include <QHttpServer>
#include <QTcpServer>
#include <QStringList>
#include <QNetworkAccessManager>



class GpxFileServer {
private:
	QHttpServer server;
	QTcpServer* tcpServer;
	QNetworkAccessManager networkAccessManager;
	
	QStringList gpxFilePaths;
	
public:
	GpxFileServer();
	
	void startServer();
	
	const QString ip;
	static const int port = 51673;
	
private:
	static void setHttpHeadersFor(QHttpServerResponse& serverResponse);
	QString getPublicIp();
};



#endif // GPX_FILE_SERVER_H
