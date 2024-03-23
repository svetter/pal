#include "dual_enum_filter_box.h"

#include "src/data/enum_names.h"

#include <QAbstractItemView>



DualEnumFilterBox::DualEnumFilterBox(QWidget* parent, const QString& title, const QList<QPair<QString, QStringList>>& entries, DualEnumFilter& filter) :
	FilterBox(parent, DualEnum, title, filter),
	filter(filter),
	comboDiscerning(new QComboBox(this)),
	comboDependent(new QComboBox(this)),
	entries(entries)
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
	
	connect(comboDiscerning,	&QComboBox::currentIndexChanged,	this,	&DualEnumFilterBox::handle_discerningComboChanged);
	connect(comboDependent,		&QComboBox::currentIndexChanged,	this,	&DualEnumFilterBox::updateFilterTypeSpecific);
	
	comboDiscerning->setCurrentIndex(filter.discerningValue);
	comboDependent->setCurrentIndex(filter.dependentValue);
}

DualEnumFilterBox::~DualEnumFilterBox()
{
	// Widgets deleted by layout
}



void DualEnumFilterBox::updateFilterTypeSpecific()
{
	filter.discerningValue = comboDiscerning->currentIndex();
	filter.dependentValue = comboDependent->currentIndex();
	emit filterChanged();
}



const Filter& DualEnumFilterBox::getFilter() const
{
	return filter;
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
	
	updateFilterTypeSpecific();
}
