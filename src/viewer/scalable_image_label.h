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
 * This file declares the ScalableImageLabel class.
 */

#ifndef SCALABLE_IMAGE_LABEL_H
#define SCALABLE_IMAGE_LABEL_H

#include <QtWidgets>



/**
 * A QLabel that displays an image and allows to zoom and move it using the mouse.
 * 
 * Must be used inside a QScrollArea.
 * 
 * Images are set using setImage() and cleared using clearImage().
 * No further work is required from the outside to enable zooming
 * and moving.
 */
class ScalableImageLabel : public QLabel
{
	Q_OBJECT
	
	/** The parent QScrollArea. */
	QScrollArea* parent;
	/** The image to display in full available resolution. */
	QPixmap fullSizePixmap;
	/** Indicates whether an image is currently loaded and displayed. */
	bool imageLoaded;
	/**
	 * Indicates whether the image should be scaled to fill the available space.
	 * This mode is enabled by default, whenever the full-size image is smaller than the available
	 * space and when the user has zoomed all the way back out.
	 */
	bool fillMode;
	
	/** The location where the mouse was last registered after being pressed over the image. Used for dragging. */
	QPoint mousePressedAt;
	
	/** The factor by which to zoom in or out when the mouse wheel is rotated. */
	constexpr static const qreal ZOOM_FACTOR = 1.2;
	
public:
	ScalableImageLabel(QScrollArea* parent);
	
public slots:
	void setImage(const QImage& image);
	void clearImage();
	
private slots:
	void wheelEvent(QWheelEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	// Drag image
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	
private:
	int getScrollX();
	int getScrollY();
	QPoint getScroll();
	void setScroll(int scrollX, int scrollY);
	void scrollRelative(QPoint scrollRel);
	void setMaxScroll(int maxScrollX, int maxScrollY);
	
	void setNormalCursor();
	void setHandCursor(bool openNotClosed);
};



#endif // SCALABLE_IMAGE_LABEL_H
