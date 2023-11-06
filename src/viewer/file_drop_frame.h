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
 * @file file_drop_frame.h
 * 
 * This file defines the FileDropFrame class.
 */

#ifndef FILE_DROP_FRAME_H
#define FILE_DROP_FRAME_H

#include <QFrame>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>



/**
 * A QFrame that accepts dropped files and emits a signal with the dropped filepaths.
 */
class FileDropFrame : public QFrame
{
	Q_OBJECT
	
public:
	/**
	 * Creates a new FileDropFrame.
	 * 
	 * @param parent	The parent widget.
	 * @param f			The window flags.
	 */
	inline FileDropFrame(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()) : QFrame(parent, f)
	{}

	/**
	 * Destroys the FileDropFrame.
	 */
	inline ~FileDropFrame()
	{}
	

public slots:
	/**
	 * Event handler for a beginning drag event.
	 * 
	 * If the drag event contains a URI list and a link action is possible, sets the drop action to
	 * link and accepts the event.
	 * 
	 * @param event	The drag event.
	 */
	inline void dragEnterEvent(QDragEnterEvent* event) override
	{
		if (!event->mimeData()->hasFormat("text/uri-list")) return;
		if (!(event->possibleActions() & Qt::LinkAction)) return;
		event->setDropAction(Qt::LinkAction);
		event->accept();
	}
	
	/**
	 * Event handler for a drop event.
	 * 
	 * Extracts a list of absolute file paths from the MIME data and emits the filesDropped signal.
	 * 
	 * @pre The drop event contains a URI list.
	 * 
	 * @param event	The drop event.
	 */
	inline void dropEvent(QDropEvent* event) override
	{
		QStringList urls = event->mimeData()->text().split("\n");
		urls.removeAll("");	// Remove empty
		
		QStringList filepaths = QStringList();
		for (const QString& url : urls) {
			filepaths.append(QFileInfo(QUrl(url).toLocalFile()).absoluteFilePath());
		}
		
		emit filesDropped(filepaths);
	}
	
signals:
	/**
	 * Emitted when files are dropped onto the FileDropFrame.
	 * 
	 * @param filepaths	A list of absolute file paths for all dropped files.
	 */
	void filesDropped(QStringList filepaths);
};



#endif // FILE_DROP_FRAME_H
