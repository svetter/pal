#include "filter_box.h"

#include <QStyle>



FilterBox::FilterBox(QWidget* parent, DataType type, const QString& title) :
	QGroupBox(parent),
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
	
	FilterBox::reset();
}

FilterBox::~FilterBox()
{
	removeButton->deleteLater();
}



void FilterBox::reset() {
	includeRadiobutton->setChecked(true);
	excludeRadiobutton->setChecked(false);
}



void FilterBox::updateFilter()
{
	getFilter()->setEnabled(isChecked());
	getFilter()->setInverted(excludeRadiobutton->isChecked());
	emit filterChanged();
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



void FilterBox::positionRemoveButton()
{
	const int removeButtonX = frameGeometry().right()   - removeButton->width()  / 2;
	const int removeButtonY = frameGeometry().top() + 9 - removeButton->height() / 2;
	removeButton->move(removeButtonX, removeButtonY);
	removeButton->setVisible(true);
}
