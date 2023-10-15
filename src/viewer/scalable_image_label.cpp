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

#include "scalable_image_label.h"



ScalableImageLabel::ScalableImageLabel(QScrollArea* parent) : QLabel(parent), parent(parent), fillMode(true)
{
	setAlignment(Qt::AlignCenter);
}



void ScalableImageLabel::setImage(const QImage& image)
{
	fullSizePixmap = QPixmap::fromImage(image);
	fillMode = true;
	setPixmap(fullSizePixmap);
}



void ScalableImageLabel::wheelEvent(QWheelEvent* event)
{
	QSize availableArea = parent->maximumViewportSize();
	int oldScrollX		= parent->horizontalScrollBar()->value();
	int oldScrollY		= parent->  verticalScrollBar()->value();
	// Mouse position here is measured from the top left corner of the available area in displayed pixels
	int mousePositionX	= event->position().toPoint().x() - oldScrollX;
	int mousePositionY	= event->position().toPoint().y() - oldScrollY;
	int oldImageWidth	= pixmap().width();
	int oldImageHeight	= pixmap().height();
	
	// Zoom factor 1 means image is fit to screen
	qreal currentZoomX	= (qreal) pixmap().width()  / availableArea.width();
	qreal currentZoomY	= (qreal) pixmap().height() / availableArea.height();
	bool zoomInNotOut = event->angleDelta().y() > 0;
	qreal factor = zoomInNotOut ? ZOOM_FACTOR : (1 / ZOOM_FACTOR);
	
	qreal newImageWidth		= fmin(fullSizePixmap.width(),  availableArea.width()  * currentZoomX * factor);
	qreal newImageHeight	= fmin(fullSizePixmap.height(), availableArea.height() * currentZoomY * factor);
	
	if (newImageWidth <= availableArea.width() && newImageHeight <= availableArea.height()) {
		newImageWidth	= availableArea.width();
		newImageHeight	= availableArea.height();
		fillMode = true;
	} else {
		fillMode = false;
	}
	
	// Rescale image
	setPixmap(fullSizePixmap.scaled(newImageWidth, newImageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	
	// Keep image centered around mouse position
	if (!fillMode) {
		qreal relMousePositionX = (qreal) (oldScrollX + mousePositionX) / oldImageWidth;
		if (oldImageWidth < availableArea.width()) {
			qreal halfWidthDiff = (qreal) (availableArea.width() - oldImageWidth) / 2;
			relMousePositionX = (qreal) (oldScrollX + mousePositionX - halfWidthDiff) / oldImageWidth;
			relMousePositionX = fmax(0, fmin(1, relMousePositionX));
		}
		qreal relMousePositionY = (qreal) (oldScrollY + mousePositionY) / oldImageHeight;
		if (oldImageHeight < availableArea.height()) {
			qreal halfHeightDiff = (qreal) (availableArea.height() - oldImageHeight) / 2;
			relMousePositionY = (qreal) (oldScrollY + mousePositionY - halfHeightDiff) / oldImageHeight;
			relMousePositionY = fmax(0, fmin(1, relMousePositionY));
		}
		assert(relMousePositionX >= 0 && relMousePositionX <= 1);
		assert(relMousePositionY >= 0 && relMousePositionY <= 1);
		
		int newScrollX = (relMousePositionX * newImageWidth ) - mousePositionX;
		int newScrollY = (relMousePositionY * newImageHeight) - mousePositionY;
		
		// Set new scroll boundaries and values
		int newMaxScrollX = newImageWidth  - availableArea.width();
		int newMaxScrollY = newImageHeight - availableArea.height();
		parent->horizontalScrollBar()->setMaximum(newMaxScrollX);
		parent->  verticalScrollBar()->setMaximum(newMaxScrollY);
		parent->horizontalScrollBar()->setValue(newScrollX);
		parent->  verticalScrollBar()->setValue(newScrollY);
	}
	
	event->accept();
}



void ScalableImageLabel::paintEvent(QPaintEvent* event)
{
	QSize availableArea = parent->maximumViewportSize();
	bool resize = false;

	// Resize if full-size image is smaller than the available area
	resize |= fullSizePixmap.width() < availableArea.width() && fullSizePixmap.height() < availableArea.height();
	// Resize if fill mode is active but the image isn't fit to the available area
	resize |= fillMode && (pixmap().width() != availableArea.width() && pixmap().height() != availableArea.height());
	// Resize if fill mode is inactive but the image is smaller than the available area
	resize |= !fillMode && (pixmap().width() < availableArea.width() && pixmap().height() < availableArea.height());
	
	if (resize) {
		setPixmap(fullSizePixmap.scaled(availableArea, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		fillMode = true;
	}
	
	QLabel::paintEvent(event);
}
