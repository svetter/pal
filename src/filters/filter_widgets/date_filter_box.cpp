#include "date_filter_box.h"



DateFilterBox::DateFilterBox(QWidget* parent, const QString& title, DateFilter& filter) :
	FilterBox(parent, Date, title, filter),
	filter(filter),
	minDateWidget(new QDateEdit(this)),
	maxDateWidget(new QDateEdit(this)),
	setMaxDateCheckbox(new QCheckBox(this)),
	spacer(new QSpacerItem(5, 0, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
	minDateWidget->setObjectName("minDateWidget");
	minDateWidget->setMinimumSize(QSize(80, 0));
	minDateWidget->setCalendarPopup(true);
	
	setMaxDateCheckbox->setObjectName("setMaxDateCheckbox");
	setMaxDateCheckbox->setToolTip(tr("Set a maximum date"));
	
	maxDateWidget->setObjectName("maxDateWidget");
	maxDateWidget->setMinimumSize(QSize(80, 0));
	maxDateWidget->setCalendarPopup(true);
	
	filterLayout->addWidget(minDateWidget);
	filterLayout->addItem(spacer);
	filterLayout->addWidget(setMaxDateCheckbox);
	filterLayout->addWidget(maxDateWidget);
	
	connect(minDateWidget,		&QDateEdit::dateChanged,	this,	&DateFilterBox::handle_minDateChanged);
	connect(setMaxDateCheckbox,	&QCheckBox::stateChanged,	this,	&DateFilterBox::handle_setMaxDateChanged);
	connect(maxDateWidget,		&QDateEdit::dateChanged,	this,	&DateFilterBox::handle_maxDateChanged);
	
	minDateWidget->setDate(filter.min);
	maxDateWidget->setDate(filter.max);
	setMaxDateCheckbox->setChecked(filter.min < filter.max);
}

DateFilterBox::~DateFilterBox()
{
	// Widgets deleted by layout
}



void DateFilterBox::updateFilterTypeSpecific()
{
	filter.min = minDateWidget->date();
	filter.max = maxDateWidget->date();
	emit filterChanged();
}



const Filter& DateFilterBox::getFilter() const
{
	return filter;
}



void DateFilterBox::handle_minDateChanged()
{
	if (!setMaxDateCheckbox->isChecked()) {
		maxDateWidget->setDate(minDateWidget->date());
	}
	else if (minDateWidget->date() > maxDateWidget->date()) {
		maxDateWidget->setDate(minDateWidget->date());
	}
	
	updateFilterTypeSpecific();
}

void DateFilterBox::handle_maxDateChanged()
{
	if (minDateWidget->date() > maxDateWidget->date()) {
		return minDateWidget->setDate(maxDateWidget->date());
	}
	
	updateFilterTypeSpecific();
}

void DateFilterBox::handle_setMaxDateChanged()
{
	maxDateWidget->setEnabled(setMaxDateCheckbox->isChecked());
	handle_minDateChanged();
}
