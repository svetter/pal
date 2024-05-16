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

#include "filter_box.h"

#include <QStyle>



FilterBox::FilterBox(QWidget* parent, DataType type, const QString& title, Filter& filter) :
	QGroupBox(parent),
	genericFilter(filter),
	type(type),
	title(title),
	removeButton(new QToolButton(parent)),
	invertButtonGroup(QButtonGroup())
{
	setupUi(this);
	
	setTitle(title);
	
	invertButtonGroup.addButton(includeRadiobutton);
	invertButtonGroup.addButton(excludeRadiobutton);
	
	removeButton->setObjectName("filterRemoveButton");
	removeButton->setIcon(style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
	removeButton->setFixedSize(12, 12);
	
	connect(this,				&QGroupBox::toggled,			this,	&FilterBox::updateFilter);
	connect(&invertButtonGroup,	&QButtonGroup::buttonClicked,	this,	&FilterBox::updateFilter);
	connect(removeButton,		&QToolButton::clicked,			this,	&FilterBox::removeRequested);
	
	setChecked(filter.isEnabled());
	excludeRadiobutton->setChecked(filter.isInverted());
}

FilterBox::~FilterBox()
{
	removeButton->deleteLater();
	delete &genericFilter;
}



void FilterBox::updateFilter()
{
	genericFilter.setEnabled(isChecked());
	genericFilter.setInverted(excludeRadiobutton->isChecked());
	emit filterChanged();
}



void FilterBox::showEvent(QShowEvent* event)
{
	QGroupBox::showEvent(event);
	positionRemoveButton();
}

void FilterBox::resizeEvent(QResizeEvent* event)
{
	QGroupBox::resizeEvent(event);
	positionRemoveButton();
}

void FilterBox::moveEvent(QMoveEvent* event)
{
	QGroupBox::moveEvent(event);
	positionRemoveButton();
}

void FilterBox::changeEvent(QEvent* event)
{
	QGroupBox::changeEvent(event);
	if (event->type() == QEvent::EnabledChange) {
		removeButton->setEnabled(isEnabled());
	}
}



void FilterBox::positionRemoveButton()
{
	const int x = parentWidget()->pos().x() + geometry().right()   - removeButton->width()  / 2;
	const int y = parentWidget()->pos().y() + geometry().top() + 9 - removeButton->height() / 2;
	removeButton->move(x, y);
	removeButton->setVisible(true);
}
