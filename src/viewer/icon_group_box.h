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
 * @file icon_group_box.h
 * 
 * This file defines the IconGroupBox class.
 */

#ifndef ICON_GROUP_BOX_H
#define ICON_GROUP_BOX_H

#include <QGroupBox>
#include <QIcon>
#include <QPainter>
#include <QHBoxLayout>



/**
 * A QGroupBox with an optional icon near the right upper corner.
 */
class IconGroupBox : public QGroupBox
{
	Q_OBJECT
	
	/** The icon to display. */
	QIcon icon;
	/** The size at which to display the icon. */
	QSize iconSize;
	
public:
	/**
	 * Creates a new IconGroupBox with the given parent.
	 * 
	 * @param parent	The parent widget.
	 */
	inline IconGroupBox(QWidget* parent = nullptr) : QGroupBox(parent)
	{}
	
	/**
	 * Sets the icon to display and the maximum size at which to display it.
	 * 
	 * @param icon		The icon to display.
	 * @param width		The maximum width with which to display the icon.
	 * @param height	The maimum height with which to display the icon.
	 */
	inline void setIcon(const QIcon& icon, int width, int height)
	{
		this->icon = icon;
		iconSize = QSize(width, height);
		update();
	}
	
protected:
	/**
	 * Paints the group box and the icon.
	 */
	inline void paintEvent(QPaintEvent* event) override
	{
		QGroupBox::paintEvent(event);
		
		if (!icon.isNull()) {
			QPainter painter(this);
			painter.setRenderHint(QPainter::SmoothPixmapTransform);
			
			int x = width() - iconSize.width() - 15;
			int y = 0;
			painter.fillRect(x - 5, y, iconSize.width() + 10, iconSize.height(), painter.background());
			icon.paint(&painter, QRect(x, y, iconSize.width(), iconSize.height()));
		}
	}
};



#endif // ICON_GROUP_BOX_H
