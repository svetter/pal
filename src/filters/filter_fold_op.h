#ifndef FILTER_FOLD_OP_H
#define FILTER_FOLD_OP_H

#include <QStringList>



enum FilterFoldOp {
	FilterFoldOp_Count,
	FilterFoldOp_Max,
	FilterFoldOp_Min,
	FilterFoldOp_Sum,
	FilterFoldOp_Average,
	FilterFoldOp_StringList
};



/**
 * A struct containing the names of the filter fold operations and providing methods to convert
 * between names and the FilterFoldOp enum.
 */
struct FilterFoldOpNames
{
	/** A list of the names of the filter fold operations, in the order of the DataType enum. */
	inline static const QStringList foldOpNames = {
		"Count", "Max", "Min", "Sum", "Average", "StringList"
	};
	
	/**
	 * Returns the name of the given filter fold operation or an empty string if the type is
	 * invalid.
	 * 
	 * @param type	The filter fold operation.
	 */
	inline static QString getName(const FilterFoldOp foldOp)
	{
		if (foldOp < 0 || foldOp >= foldOpNames.size()) return QString();
		return foldOpNames.at(foldOp);
	}
	
	/**
	 * Returns the filter fold operation corresponding to the given name or -1 if the name is
	 * invalid.
	 * 
	 * @param name	The name of the filter fold operation.
	 */
	inline static FilterFoldOp getFoldOp(const QString& name)
	{
		if (!foldOpNames.contains(name)) return FilterFoldOp(-1);
		return FilterFoldOp(foldOpNames.indexOf(name));
	}
};



#endif // FILTER_FOLD_OP_H
