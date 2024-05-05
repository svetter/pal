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
