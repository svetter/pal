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



ScalableImageLabel::ScalableImageLabel(QScrollArea* parent) : QLabel(parent), parent(parent), imageLoaded(false), fillMode(true), mousePressedAt(QPoint())
{
	setAlignment(Qt::AlignCenter);
}



void ScalableImageLabel::setImage(const QImage& image)
{
	fullSizePixmap = QPixmap::fromImage(image);
	fillMode = true;
	setPixmap(fullSizePixmap);
	imageLoaded = true;
	setHandCursor(true);
}

void ScalableImageLabel::clearImage()
{
	fullSizePixmap = QPixmap();
	setPixmap(QPixmap());
	imageLoaded = false;
	setNormalCursor();
}



void ScalableImageLabel::wheelEvent(QWheelEvent* event)
{
	QSize availableArea = parent->maximumViewportSize();
	QPoint oldScroll = getScroll();
	// Mouse position here is measured from the top left corner of the available area in displayed pixels
	QPoint mousePosition = event->position().toPoint() - oldScroll;
	QSize oldImageSize = pixmap().size();
	
	// Zoom factor 1 means image is fit to screen
	qreal currentZoomX	= (qreal) oldImageSize.width()  / availableArea.width();
	qreal currentZoomY	= (qreal) oldImageSize.height() / availableArea.height();
	bool zoomInNotOut = event->angleDelta().y() > 0;
	qreal factor = zoomInNotOut ? ZOOM_FACTOR : (1 / ZOOM_FACTOR);
	
	int newImageWidth	= fmin(fullSizePixmap.width(),  availableArea.width()  * currentZoomX * factor);
	int newImageHeight	= fmin(fullSizePixmap.height(), availableArea.height() * currentZoomY * factor);
	
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
		qreal relMousePositionX = (qreal) (oldScroll.x() + mousePosition.x()) / oldImageSize.width();
		if (oldImageSize.width() < availableArea.width()) {
			qreal halfWidthDiff = (qreal) (availableArea.width() - oldImageSize.width()) / 2;
			relMousePositionX = (qreal) (oldScroll.x() + mousePosition.x() - halfWidthDiff) / oldImageSize.width();
			relMousePositionX = fmax(0, fmin(1, relMousePositionX));
		}
		qreal relMousePositionY = (qreal) (oldScroll.y() + mousePosition.y()) / oldImageSize.height();
		if (oldImageSize.height() < availableArea.height()) {
			qreal halfHeightDiff = (qreal) (availableArea.height() - oldImageSize.height()) / 2;
			relMousePositionY = (qreal) (oldScroll.y() + mousePosition.y() - halfHeightDiff) / oldImageSize.height();
			relMousePositionY = fmax(0, fmin(1, relMousePositionY));
		}
		assert(relMousePositionX >= 0 && relMousePositionX <= 1);
		assert(relMousePositionY >= 0 && relMousePositionY <= 1);
		
		int newScrollX = (relMousePositionX * newImageWidth ) - mousePosition.x();
		int newScrollY = (relMousePositionY * newImageHeight) - mousePosition.y();
		
		// Set new scroll boundaries and values
		int newMaxScrollX = newImageWidth  - availableArea.width();
		int newMaxScrollY = newImageHeight - availableArea.height();
		setMaxScroll(newMaxScrollX, newMaxScrollY);
		setScroll(newScrollX, newScrollY);
	}
	
	event->accept();
}



void ScalableImageLabel::paintEvent(QPaintEvent* event)
{
	if (!imageLoaded) return;
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



void ScalableImageLabel::mousePressEvent(QMouseEvent* event)
{
	if (!imageLoaded) return;
	setHandCursor(false);
	if (fillMode) return;
	mousePressedAt = event->globalPosition().toPoint();
}

void ScalableImageLabel::mouseMoveEvent(QMouseEvent* event)
{
	if (!imageLoaded || fillMode || mousePressedAt.isNull()) return;
	QPoint mouseMove = event->globalPosition().toPoint() - mousePressedAt;
	scrollRelative(mouseMove);
	mousePressedAt = event->globalPosition().toPoint();
}

void ScalableImageLabel::mouseReleaseEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	if (!imageLoaded) return;
	setHandCursor(true);
	if (fillMode) return;
	mousePressedAt = QPoint();
}



int ScalableImageLabel::getScrollX()
{
	return parent->horizontalScrollBar()->value();
}

int ScalableImageLabel::getScrollY()
{
	return parent->verticalScrollBar()->value();
}

QPoint ScalableImageLabel::getScroll()
{
	return QPoint(getScrollX(), getScrollY());
}

void ScalableImageLabel::setScroll(int scrollX, int scrollY)
{
	parent->horizontalScrollBar()->setValue(scrollX);
	parent->  verticalScrollBar()->setValue(scrollY);
}

void ScalableImageLabel::scrollRelative(QPoint scrollRel)
{
	setScroll(getScrollX() - scrollRel.x(), getScrollY() - scrollRel.y());
}

void ScalableImageLabel::setMaxScroll(int maxScrollX, int maxScrollY)
{
	parent->horizontalScrollBar()->setMaximum(maxScrollX);
	parent->verticalScrollBar()->setMaximum(maxScrollY);
}



void ScalableImageLabel::setNormalCursor()
{
	parent->viewport()->setProperty("cursor", QVariant(QCursor(Qt::ArrowCursor)));
}

void ScalableImageLabel::setHandCursor(bool openNotClosed)
{
	Qt::CursorShape cursorShape = openNotClosed ? Qt::OpenHandCursor : Qt::ClosedHandCursor;
	parent->viewport()->setProperty("cursor", QVariant(QCursor(cursorShape)));
}
