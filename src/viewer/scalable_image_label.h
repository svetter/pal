/*
 * Copyright 2023-2024 Simon Vetter
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
	/** Indicates whether an image is currently loaded and displayed. */
	bool imageLoaded;
	/** The image to display in full available resolution. */
	QPixmap fullSizePixmap;
	/**
	 * Indicates whether the image should be scaled to fill the available space.
	 * This mode is enabled by default, whenever the full-size image is smaller than the available
	 * space and when the user has zoomed all the way back out.
	 */
	bool fillMode;
	
	/** When fill mode is inactive, indicates where in the current size of the image the center of the displayed area is. */
	QPoint imageCenter;
	/** The location where the mouse was last registered after being pressed over the image. Used for dragging. */
	QPoint mousePressedAt;
	
	/** The factor by which to zoom in or out when the mouse wheel is rotated. */
	constexpr static const qreal ZOOM_FACTOR = 1.2;
	/** The maximum ratio between the displayed size of the image and its original size, e.g., 200% = 2. */
	constexpr static const qreal MAX_ZOOM_RATIO = 2;
	
public:
	ScalableImageLabel(QScrollArea* parent);
	
public slots:
	void setImage(const QImage& image);
	void clearImage();
	
private slots:
	// Zoom
	void wheelEvent(QWheelEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	// Resize
	void paintEvent(QPaintEvent* event) override;
	// Drag image
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	
signals:
	/**
	 * Emitted when the user has interacted with the image by clicking or scrolling the mouse.
	 */
	void userInteracted();
	
	
private:
	void saveNewImageCenter();
	
	void setBarsEnabled(bool enabled) const;
	int getScrollX() const;
	int getScrollY() const;
	QPoint getScroll() const;
	void setScroll(int scrollX, int scrollY) const;
	void scrollRelative(QPoint scrollRel) const;
	void setMaxScroll(int maxScrollX, int maxScrollY) const;
	
	void setNormalCursor() const;
	void setHandCursor(bool openNotClosed) const;
};



#endif // SCALABLE_IMAGE_LABEL_H
