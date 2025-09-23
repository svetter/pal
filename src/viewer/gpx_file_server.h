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
#include <QFile>



class GpxFileServer {
private:
	QHttpServer server;
	QTcpServer* tcpServer;
	
	QFile* currentFile;
	int currentFileIndex;
	
public:
	const QString ip;
	static const int port = 51673;
	
	GpxFileServer();
	virtual ~GpxFileServer();
	
	void setup();
	QString serveNewFile(const QString& filepath);
	void stop();
	
private:
	QString getCurrentServersideFilename() const;
	static void setHttpHeadersFor(QHttpServerResponse& serverResponse);
	static QString getPublicIp();
};



#endif // GPX_FILE_SERVER_H
