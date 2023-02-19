#ifndef PEAKS_TABLE_H
#define PEAKS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/peak.h"

#include <QWidget>



class PeaksTable : public NormalTable {
	Q_OBJECT
	
public:
	const Column* nameColumn;
	const Column* heightColumn;
	const Column* volcanoColumn;
	const Column* regionIDColumn;
	const Column* mapsLinkColumn;
	const Column* earthLinkColumn;
	const Column* wikiLinkColumn;
	
	PeaksTable(const Column* foreignRegionIDColumn);
	
	int addRow(QWidget* parent, const Peak* peak);
	void updateRow(QWidget* parent, ValidItemID peakID, const Peak* peak);
	
	QList<QVariant> mapDataToQVariantList(const Peak* peak) const;
	
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // PEAKS_TABLE_H
