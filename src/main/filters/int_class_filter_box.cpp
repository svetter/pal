#include "int_class_filter_box.h"



IntClassFilterBox::IntClassFilterBox(QWidget* parent, const QString& title, int classIncrement, int minValue, int maxValue) :
	FilterBox(parent, title),
	classIncrement(classIncrement),
	minValue(minValue),
	maxValue(maxValue),
	minCombo(new QComboBox(this)),
	maxCombo(new QComboBox(this)),
	setMaxCheckbox(new QCheckBox(this)),
	spacer(new QSpacerItem(5, 0, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
	connect(minCombo,		&QComboBox::currentIndexChanged,	this,	&IntClassFilterBox::handle_minChanged);
	connect(setMaxCheckbox,	&QCheckBox::stateChanged,			this,	&IntClassFilterBox::handle_setMaxChanged);
	connect(maxCombo,		&QComboBox::currentIndexChanged,	this,	&IntClassFilterBox::handle_maxChanged);
	
	IntClassFilterBox::setup();
	IntClassFilterBox::reset();
}

IntClassFilterBox::~IntClassFilterBox()
{}



void IntClassFilterBox::setup()
{
	int value = (minValue / classIncrement + 1) * classIncrement;
	QStringList minComboEntries = { "<" + QString::number(value) };
	QStringList maxComboEntries = { QString::number(value - 1) };
	while (value <= maxValue) {
		minComboEntries.append(QString::number(value));
		value += classIncrement;
		maxComboEntries.append(QString::number(value - 1));
	}
	
	minCombo->setObjectName("minCombo");
	minCombo->addItems(minComboEntries);
	
	setMaxCheckbox->setObjectName("setMaxCheckbox");
	setMaxCheckbox->setToolTip(tr("Set a maximum class"));
	
	maxCombo->setObjectName("maxCombo");
	maxCombo->setEnabled(false);
	maxCombo->addItems(maxComboEntries);
	
	filterLayout->addWidget(minCombo);
	filterLayout->addItem(spacer);
	filterLayout->addWidget(setMaxCheckbox);
	filterLayout->addWidget(maxCombo);
}

void IntClassFilterBox::reset()
{
	FilterBox::reset();
	
	minCombo		->setCurrentIndex(0);
	maxCombo		->setCurrentIndex(0);
	setMaxCheckbox	->setChecked(false);
}



void IntClassFilterBox::handle_minChanged()
{
	if (!setMaxCheckbox->isChecked()) {
		maxCombo->setCurrentIndex(minCombo->currentIndex());
	}
	else if (minCombo->currentIndex() > maxCombo->currentIndex()) {
		maxCombo->setCurrentIndex(minCombo->currentIndex());
	}
	
	emit filterChanged();
}

void IntClassFilterBox::handle_maxChanged()
{
	if (minCombo->currentIndex() > maxCombo->currentIndex()) {
		return minCombo->setCurrentIndex(maxCombo->currentIndex());
	}
	
	emit filterChanged();
}

void IntClassFilterBox::handle_setMaxChanged()
{
	maxCombo->setEnabled(setMaxCheckbox->isChecked());
	handle_minChanged();
}
