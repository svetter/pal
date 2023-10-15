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

#ifndef SCALABLE_IMAGE_LABEL_H
#define SCALABLE_IMAGE_LABEL_H

#include <QtWidgets>



class ScalableImageLabel : public QLabel
{
	Q_OBJECT
	
	QPixmap fullSizePixmap;
	QScrollArea* parent;
	bool fillMode;
	
	constexpr static const qreal ZOOM_FACTOR = 1.2;
	
public:
	ScalableImageLabel(QScrollArea* parent);
	
public slots:
	void setImage(const QImage& image);
	
private slots:
	void wheelEvent(QWheelEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
};



#endif // SCALABLE_IMAGE_LABEL_H
