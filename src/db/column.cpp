#include "column.h"

#include "table.h"
#include "database.h"

#include <QCoreApplication>



Column::Column(QString name, QString uiName, DataType type, bool nullable, bool primaryKey, const Column* foreignKey, const Table* table) :
		name(name),
		uiName(uiName),
		type(type),
		primaryKey(primaryKey),
		foreignKey(foreignKey),
		nullable(nullable),
		table(table)
{
	assert(name.compare(QString("ID"), Qt::CaseInsensitive) != 0);
	assert(table->isAssociative == (primaryKey && foreignKey));
	if (primaryKey)					assert(!nullable);
	if (primaryKey || foreignKey)	assert(type == integer && name.endsWith("ID"));
	if (name.endsWith("ID"))		assert(primaryKey || foreignKey);
}



bool Column::isPrimaryKey() const
{
	return primaryKey;
}

bool Column::isForeignKey() const
{
	return foreignKey;
}

const Column* Column::getReferencedForeignColumn() const
{
	return foreignKey;
}

int Column::getIndex() const
{
	return table->getColumnIndex(this);
}



QString getColumnListStringOf(QList<const Column*> columns)
{
	QString result = "";
	bool first = true;
	for (auto iter = columns.begin(); iter != columns.end(); iter++) {
		result = result + (first ? "" : ", ") + (*iter)->name;
		first = false;
	}
	return result;
}



QString getTranslatedWhatIfDeleteResultDescription(const WhatIfDeleteResult& whatIfResult)
{
	int numAffectedItems = whatIfResult.numAffectedRowIndices;
	QString itemName;
	if (numAffectedItems == 1) {
		itemName = whatIfResult.itemTable->getItemNameSingularLowercase();
	} else {
		itemName = whatIfResult.itemTable->getItemNamePluralLowercase();
	}
	/*: This will be part of a listing of consequences of deleting an item.
	 *  An example would be: "Hiker will be removed from 42 ascents."
	 *  Here, the string "255 ascents" would have been the result of using the base string "%1 %2"
	 *  and replacing "%1" with "42" and "%2% with "ascents".
	 *  A translation for %2 is retrieved from elsewhere.
	 */
	QString baseString = QCoreApplication::translate("WhatIfDeleteResult", "%1 %2");
	return baseString.arg(numAffectedItems).arg(itemName);
}

QString getTranslatedWhatIfDeleteResultDescription(const QList<WhatIfDeleteResult>& whatIfResults)
{
	QString baseString = QCoreApplication::translate("WhatIfDeleteResult", "The item will be removed from %1.");
	//: This goes in between any two entries in a list, but not before the last entry
	QString listSeparatorString = QCoreApplication::translate("WhatIfDeleteResult", ", ");
	//: This goes in between the last and second-to-last entries in a list
	QString oxfordCommaString = QCoreApplication::translate("WhatIfDeleteResult", ", and ");
	
	QString argumentString = "";
	int iterationsLeft = whatIfResults.size();
	for (auto iter = whatIfResults.constBegin(); iter != whatIfResults.constEnd(); iter++) {
		argumentString.append(getTranslatedWhatIfDeleteResultDescription(*iter));
		
		switch (--iterationsLeft) {
		case 0:
			break;
		case 1:
			argumentString.append(oxfordCommaString);
			break;
		default:
			assert(iterationsLeft > 0);
			argumentString.append(listSeparatorString);
		}
	}
	
	return baseString.arg(argumentString);
}
