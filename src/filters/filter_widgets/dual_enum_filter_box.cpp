#include "dual_enum_filter_box.h"

#include "src/data/enum_names.h"

#include <QAbstractItemView>



DualEnumFilterBox::DualEnumFilterBox(QWidget* parent, const QString& title, const QList<QPair<QString, QStringList>>& entries) :
	FilterBox(parent, DualEnum, title),
	comboDiscerning(new QComboBox(this)),
	comboDependent(new QComboBox(this)),
	entries(entries)
{
	connect(comboDiscerning,	&QComboBox::currentIndexChanged,	this,	&DualEnumFilterBox::handle_discerningComboChanged);
	connect(comboDependent,		&QComboBox::currentIndexChanged,	this,	&DualEnumFilterBox::filterChanged);
	
	DualEnumFilterBox::setup();
	DualEnumFilterBox::reset();
}

DualEnumFilterBox::~DualEnumFilterBox()
{}



void DualEnumFilterBox::setup()
{
	comboDiscerning->setObjectName("comboDiscerning");
	comboDiscerning->setMaximumWidth(150);
	comboDiscerning->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	comboDiscerning->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
	comboDiscerning->view()->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	
	QStringList discerningEntries = QStringList();
	std::transform(
		entries.constBegin(),
		entries.constEnd(),
		std::back_inserter(discerningEntries),
		[](QPair<QString, QStringList> qPair){ return qPair.first; }
		);
	comboDiscerning->insertItems(0, EnumNames::translateList(discerningEntries));
	
	comboDependent->setObjectName("comboDependent");
	comboDependent->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	comboDependent->setMaximumWidth(150);
	comboDependent->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
	comboDependent->view()->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	
	filterLayout->addWidget(comboDiscerning);
	filterLayout->addWidget(comboDependent);
}

void DualEnumFilterBox::reset()
{
	FilterBox::reset();
	
	comboDiscerning->setCurrentIndex(0);
	comboDependent->setCurrentIndex(0);
}



void DualEnumFilterBox::handle_discerningComboChanged()
{
	const int discerningIndex = comboDiscerning->currentIndex();
	const bool dependantEnabled = discerningIndex > 0;
	comboDependent->setEnabled(dependantEnabled);
	
	comboDependent->clear();
	if (dependantEnabled) {
		QStringList translatedEntries = EnumNames::translateList(entries.at(discerningIndex).second);
		translatedEntries.removeAt(0);
		comboDependent->insertItems(0, translatedEntries);
		comboDependent->setCurrentIndex(0);
	} else {
		comboDependent->setPlaceholderText(EnumNames::translateList(entries.at(0).second).at(0));
	}
}
