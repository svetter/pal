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

#ifndef IMAGE_DISPLAY_H
#define IMAGE_DISPLAY_H

#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>



class ImageDisplay : public QLabel
{
	Q_OBJECT
	
	QPixmap imagePixmap;
	
public:
	inline explicit ImageDisplay(QWidget* parent) :
			QLabel(parent)
	{
		setMinimumSize(1, 1);
		setScaledContents(false);
	}
	
	inline virtual int heightForWidth(int width) const override
	{
		if (imagePixmap.isNull()) {
			return height();
		}
		return ((double) imagePixmap.height() * width) / imagePixmap.width();
	}
	
	inline virtual QSize sizeHint() const override
	{
		return QSize(width(), heightForWidth(width()));
	}
	
public slots:
	inline void setPixmap(const QPixmap& pixmap)
	{
		this->imagePixmap = pixmap;
		QLabel::setPixmap(scaledPixmap());
	}
	
	inline void resizeEvent(QResizeEvent* event) override
	{
		Q_UNUSED(event);
		
		if(!imagePixmap.isNull()) {
			QLabel::setPixmap(scaledPixmap());
		}
	}
	
private:
	inline QPixmap scaledPixmap() const
	{
		return imagePixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
};



#endif // IMAGE_DISPLAY_H
