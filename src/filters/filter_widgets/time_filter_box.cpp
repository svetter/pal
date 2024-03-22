#include "time_filter_box.h"



TimeFilterBox::TimeFilterBox(QWidget* parent, const QString& title, TimeFilter& filter) :
	FilterBox(parent, Time, title, filter),
	filter(filter),
	minTimeWidget(new QTimeEdit(this)),
	maxTimeWidget(new QTimeEdit(this)),
	setMaxTimeCheckbox(new QCheckBox(this)),
	spacer(new QSpacerItem(5, 0, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
	connect(minTimeWidget,		&QTimeEdit::timeChanged,	this,	&TimeFilterBox::handle_minTimeChanged);
	connect(setMaxTimeCheckbox,	&QCheckBox::stateChanged,	this,	&TimeFilterBox::handle_setMaxTimeChanged);
	connect(maxTimeWidget,		&QTimeEdit::timeChanged,	this,	&TimeFilterBox::handle_maxTimeChanged);
	
	TimeFilterBox::setup();
	TimeFilterBox::reset();
}

TimeFilterBox::~TimeFilterBox()
{
	// Widgets deleted by layout
}



void TimeFilterBox::setup()
{
	minTimeWidget->setObjectName("minTimeWidget");
	
	setMaxTimeCheckbox->setObjectName("setMaxTimeCheckbox");
	setMaxTimeCheckbox->setToolTip(tr("Set a maximum time"));
	
	maxTimeWidget->setObjectName("maxTimeWidget");
	
	filterLayout->addWidget(minTimeWidget);
	filterLayout->addItem(spacer);
	filterLayout->addWidget(setMaxTimeCheckbox);
	filterLayout->addWidget(maxTimeWidget);
}

void TimeFilterBox::reset()
{
	FilterBox::reset();
	
	minTimeWidget		->setTime(QTime(12, 00));
	maxTimeWidget		->setTime(QTime(12, 00));
	setMaxTimeCheckbox	->setChecked(true);
}



void TimeFilterBox::updateFilterTypeSpecific()
{
	filter.setMinMax(minTimeWidget->time(), maxTimeWidget->time());
	emit filterChanged();
}



const Filter& TimeFilterBox::getFilter() const
{
	return filter;
}



void TimeFilterBox::handle_minTimeChanged()
{
	if (!setMaxTimeCheckbox->isChecked()) {
		maxTimeWidget->setTime(minTimeWidget->time());
	}
	else if (minTimeWidget->time() > maxTimeWidget->time()) {
		maxTimeWidget->setTime(minTimeWidget->time());
	}
	
	updateFilterTypeSpecific();
}

void TimeFilterBox::handle_maxTimeChanged()
{
	if (minTimeWidget->time() > maxTimeWidget->time()) {
		return minTimeWidget->setTime(maxTimeWidget->time());
	}
	
	updateFilterTypeSpecific();
}

void TimeFilterBox::handle_setMaxTimeChanged()
{
	maxTimeWidget->setEnabled(setMaxTimeCheckbox->isChecked());
	handle_minTimeChanged();
}
