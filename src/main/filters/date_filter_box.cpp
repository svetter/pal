#include "date_filter_box.h"



DateFilterBox::DateFilterBox(QWidget* parent, const QString& title) :
	FilterBox(parent, title),
	minDateWidget(new QDateEdit(this)),
	maxDateWidget(new QDateEdit(this)),
	setMaxDateCheckbox(new QCheckBox(this)),
	spacer(new QSpacerItem(5, 0, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
	connect(minDateWidget,		&QDateEdit::dateChanged,	this,	&DateFilterBox::handle_minDateChanged);
	connect(setMaxDateCheckbox,	&QCheckBox::stateChanged,	this,	&DateFilterBox::handle_setMaxDateChanged);
	connect(maxDateWidget,		&QDateEdit::dateChanged,	this,	&DateFilterBox::handle_maxDateChanged);
	
	DateFilterBox::setup();
	DateFilterBox::reset();
}

DateFilterBox::~DateFilterBox()
{}



void DateFilterBox::setup()
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
}

void DateFilterBox::reset()
{
	FilterBox::reset();
	
	minDateWidget		->setDate(QDateTime::currentDateTime().date());
	maxDateWidget		->setDate(QDateTime::currentDateTime().date());
	setMaxDateCheckbox	->setChecked(true);
}



void DateFilterBox::handle_minDateChanged()
{
	if (!setMaxDateCheckbox->isChecked()) {
		maxDateWidget->setDate(minDateWidget->date());
	}
	else if (minDateWidget->date() > maxDateWidget->date()) {
		maxDateWidget->setDate(minDateWidget->date());
	}
	
	emit filterChanged();
}

void DateFilterBox::handle_maxDateChanged()
{
	if (minDateWidget->date() > maxDateWidget->date()) {
		return minDateWidget->setDate(maxDateWidget->date());
	}
	
	emit filterChanged();
}

void DateFilterBox::handle_setMaxDateChanged()
{
	maxDateWidget->setEnabled(setMaxDateCheckbox->isChecked());
	handle_minDateChanged();
}
