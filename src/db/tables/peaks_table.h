#ifndef PEAKS_TABLE_H
#define PEAKS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/peak.h"

#include <QWidget>



class PeaksTable : public NormalTable {
	Q_OBJECT
	
public:
	Column* const nameColumn;
	Column* const heightColumn;
	Column* const volcanoColumn;
	Column* const regionIDColumn;
	Column* const mapsLinkColumn;
	Column* const earthLinkColumn;
	Column* const wikiLinkColumn;
	
	PeaksTable(Column* foreignRegionIDColumn);
	
	int addRow(QWidget* parent, Peak* peak);
	void updateRow(QWidget* parent, ValidItemID peakID, const Peak* peak);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, const Peak* peak) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // PEAKS_TABLE_H
