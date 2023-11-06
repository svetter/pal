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
 * @file scalable_image_label.cpp
 *
 * This file defines the ScalableImageLabel class.
 */

#include "scalable_image_label.h"



/**
 * Creates a new ScalableImageLabel.
 * 
 * @param parent	The QScrollArea this ScalableImageLabel is contained in.
 */
ScalableImageLabel::ScalableImageLabel(QScrollArea* parent) : QLabel(parent), parent(parent), imageLoaded(false), fillMode(true), mousePressedAt(QPoint())
{
	setAlignment(Qt::AlignCenter);
}



/**
 * Sets the image to be displayed.
 * 
 * @param image	The image to be displayed, in full resolution.
 */
void ScalableImageLabel::setImage(const QImage& image)
{
	fullSizePixmap = QPixmap::fromImage(image);
	imageLoaded = true;
	setPixmap(fullSizePixmap);
	fillMode = true;
	setBarsEnabled(false);
	imageCenter = QPoint();
	setHandCursor(true);
}

/**
 * Clears the image and fully resets the ScalableImageLabel.
 */
void ScalableImageLabel::clearImage()
{
	fullSizePixmap = QPixmap();
	setPixmap(QPixmap());
	imageLoaded = false;
	imageCenter = QPoint();
	setNormalCursor();
}



/**
 * Event handler for mouse wheel events, which are interpreted as zooming in or out.
 * 
 * Zooming is done by scaling the image without cropping it. The image is then moved so that the
 * mouse points to the same position in the image as before (except when there is a letter- or
 * pillarbox).
 * 
 * Zooming is always constrained first by the available area: the image is never smaller than the
 * available area. Second, zooming is constrained by the image's resolution: the image is only
 * allowed to be scaled up above the zoom limit (MAX_ZOOM_RATIO times its original size) if the
 * size while zoomed in all the way is still smaller than the available area in both dimensions. If
 * the maximum zoomed image is larger than the available area in either dimension, the user can
 * zoom in until the image reaches MAX_ZOOM_RATIO times its size.
 * 
 * @param event	The mouse wheel event.
 */
void ScalableImageLabel::wheelEvent(QWheelEvent* event)
{
	QSize availableArea = parent->maximumViewportSize();
	QPoint oldScroll = getScroll();
	// Mouse position here is measured from the top left corner of the available area in displayed pixels
	QPoint mousePosition = event->position().toPoint() - oldScroll;
	QSize oldImageSize = pixmap().size();
	
	qreal currentZoomX	= (qreal) oldImageSize.width()  / availableArea.width();
	qreal currentZoomY	= (qreal) oldImageSize.height() / availableArea.height();
	bool zoomInNotOut = event->angleDelta().y() > 0;
	qreal factor = zoomInNotOut ? ZOOM_FACTOR : (1 / ZOOM_FACTOR);
	
	int newImageWidth	= fmin(MAX_ZOOM_RATIO * fullSizePixmap.width(),  availableArea.width()  * currentZoomX * factor);
	int newImageHeight	= fmin(MAX_ZOOM_RATIO * fullSizePixmap.height(), availableArea.height() * currentZoomY * factor);
	
	if (newImageWidth <= availableArea.width() && newImageHeight <= availableArea.height()) {
		newImageWidth	= availableArea.width();
		newImageHeight	= availableArea.height();
		fillMode = true;
		setBarsEnabled(false);
	} else {
		fillMode = false;
		setBarsEnabled(true);
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
		
		// Calculate and save new center position
		saveNewImageCenter();
	}
	else {
		imageCenter = QPoint();
	}
	
	event->accept();
}



/**
 * Event handler for paint events, which are interpreted as resizing events.
 * 
 * When the available area is resized, the image is also rescaled **iff* fill mode is active.
 * If the resize event causes the image to be smaller than the available area, fill mode is
 * activated.
 * 
 * @param event	The paint event.
 */
void ScalableImageLabel::paintEvent(QPaintEvent* event)
{
	if (!imageLoaded) return;
	QSize availableArea = parent->maximumViewportSize();
	bool resize = false;
	
	// Resize if max-zoomed image is smaller than the available area
	resize |= MAX_ZOOM_RATIO * fullSizePixmap.width() < availableArea.width() && MAX_ZOOM_RATIO * fullSizePixmap.height() < availableArea.height();
	// Resize if fill mode is active but the image is bigger than the available area
	resize |= fillMode && (pixmap().width() > availableArea.width() || pixmap().height() > availableArea.height());
	// Resize if the image is smaller than the available area
	resize |= pixmap().width() < availableArea.width() && pixmap().height() < availableArea.height();
	
	if (resize) {
		setPixmap(fullSizePixmap.scaled(availableArea, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		if (!fillMode) {
			fillMode = true;
			setBarsEnabled(false);
		}
	}
	
	if (!fillMode) {
		// Make sure image stays centered
		int moveX = getScrollX() + availableArea.width()  / 2 - imageCenter.x();
		int moveY = getScrollY() + availableArea.height() / 2 - imageCenter.y();
		if (moveX || moveY) scrollRelative(QPoint(moveX, moveY));
	}
	
	QLabel::paintEvent(event);
}



/**
 * Event handler for mouse press events, which are interpreted as the start of a drag event.
 * 
 * If an image is loaded, the cursor is changed to a closed hand cursor. Unless fill mode is
 * active, the mouse position is saved.
 * 
 * @param event	The mouse press event.
 */
void ScalableImageLabel::mousePressEvent(QMouseEvent* event)
{
	if (!imageLoaded) return;
	setHandCursor(false);
	if (fillMode) return;
	mousePressedAt = event->globalPosition().toPoint();
}

/**
 * Event handler for mouse press events, which are interpreted as a step in a drag event.
 * 
 * Event is ignored if no image is loaded or if fill mode is active. Otherwise, the image is
 * scrolled by the distance the mouse has moved since the last mouse move event. Then, the saved
 * mouse position is updated.
 *
 * @param event	The mouse move event.
 */
void ScalableImageLabel::mouseMoveEvent(QMouseEvent* event)
{
	if (!imageLoaded || fillMode || mousePressedAt.isNull()) return;
	QPoint mouseMove = event->globalPosition().toPoint() - mousePressedAt;
	scrollRelative(mouseMove);
	saveNewImageCenter();
	mousePressedAt = event->globalPosition().toPoint();
}

/**
 * Event handler for mouse release events, which are interpreted as the end of a drag event.
 * 
 * If an image is loaded, the cursor is changed back to an open hand cursor. Unless fill mode is
 * active, the saved mouse position is cleared.
 * 
 * @param event	The mouse release event.
 */
void ScalableImageLabel::mouseReleaseEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	if (!imageLoaded) return;
	setHandCursor(true);
	if (fillMode) return;
	mousePressedAt = QPoint();
}



void ScalableImageLabel::saveNewImageCenter()
{
	QSize availableArea = parent->maximumViewportSize();
	int centerX = getScrollX() + availableArea.width () / 2;
	int centerY = getScrollY() + availableArea.height() / 2;
	imageCenter = QPoint(centerX, centerY);
}


/**
 * Sets the enabled status of the parent QScrollArea's scroll bars.
 * 
 * Used to reduce flickering in the bars during resize.
 * 
 * @param enabled	The enabled status to set the parent QScrollArea to.
 */
void ScalableImageLabel::setBarsEnabled(bool enabled) const
{
	parent->horizontalScrollBar()->setEnabled(enabled);
	parent->  verticalScrollBar()->setEnabled(enabled);
}

/**
 * Returns the current horizontal scroll value of the surrounding QScrollArea.
 * 
 * @return	The current horizontal scroll value.
 */
int ScalableImageLabel::getScrollX() const
{
	return parent->horizontalScrollBar()->value();
}

/**
 * Returns the current vertical scroll value of the surrounding QScrollArea.
 * 
 * @return	The current vertical scroll value.
 */
int ScalableImageLabel::getScrollY() const
{
	return parent->verticalScrollBar()->value();
}

/**
 * Returns the current scroll value of the surrounding QScrollArea.
 * 
 * @return	The current scroll value.
 */
QPoint ScalableImageLabel::getScroll() const
{
	return QPoint(getScrollX(), getScrollY());
}

/**
 * Sets the scroll value of the surrounding QScrollArea.
 * 
 * @param scrollX	The new horizontal scroll value.
 * @param scrollY	The new vertical scroll value.
 */
void ScalableImageLabel::setScroll(int scrollX, int scrollY) const
{
	parent->horizontalScrollBar()->setValue(scrollX);
	parent->  verticalScrollBar()->setValue(scrollY);
}

/**
 * Scrolls the surrounding QScrollArea by the given amounts.
 * 
 * @param scrollRel	The scroll vector.
 */
void ScalableImageLabel::scrollRelative(QPoint scrollRel) const
{
	setScroll(getScrollX() - scrollRel.x(), getScrollY() - scrollRel.y());
}

/**
 * Sets the maximum scroll values of the surrounding QScrollArea.
 * 
 * @param maxScrollX	The new maximum horizontal scroll value.
 * @param maxScrollY	The new maximum vertical scroll value.
 */
void ScalableImageLabel::setMaxScroll(int maxScrollX, int maxScrollY) const
{
	parent->horizontalScrollBar()->setMaximum(maxScrollX);
	parent->verticalScrollBar()->setMaximum(maxScrollY);
}



/**
 * Sets the cursor to the normal arrow cursor for the surrounding QScrollArea (content area only).
 */
void ScalableImageLabel::setNormalCursor() const
{
	parent->viewport()->setProperty("cursor", QVariant(QCursor(Qt::ArrowCursor)));
}

/**
 * Sets the cursor to a hand cursor for the surrounding QScrollArea (content area only).
 * 
 * @param openNotClosed	Whether to set an open (true) or closed (false) hand cursor.
 */
void ScalableImageLabel::setHandCursor(bool openNotClosed) const
{
	Qt::CursorShape cursorShape = openNotClosed ? Qt::OpenHandCursor : Qt::ClosedHandCursor;
	parent->viewport()->setProperty("cursor", QVariant(QCursor(cursorShape)));
}
