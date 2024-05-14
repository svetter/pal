#ifndef NUMERIC_FOLD_OP_H
#define NUMERIC_FOLD_OP_H

#include <QStringList>



/**
 * The different fold operations that can be performed in a NumericFoldCompositeColumn.
 */
enum NumericFoldOp {
	AverageFold,
	SumFold,
	MaxFold,
	MinFold
};



/**
 * A struct containing the names of the fold operations and providing methods to convert between
 * names and the NumericFoldOpNames enum.
 */
struct NumericFoldOpNames
{
	/** A list of the names of the fold operations, in the order of the DataType enum. */
	inline static const QStringList foldOpNames = {
		"Average", "Sum", "Max", "Min"
	};
	
	/**
	 * Returns the name of the given fold operation or an empty string if the type is
	 * invalid.
	 * 
	 * @param foldOp	The fold operation.
	 */
	inline static QString getName(const NumericFoldOp foldOp)
	{
		if (foldOp < 0 || foldOp >= foldOpNames.size()) return QString();
		return foldOpNames.at(foldOp);
	}
	
	/**
	 * Returns the fold operation corresponding to the given name or -1 if the name is invalid.
	 * 
	 * @param name	The name of the fold operation.
	 */
	inline static NumericFoldOp getFoldOp(const QString& name)
	{
		if (!foldOpNames.contains(name)) return NumericFoldOp(-1);
		return NumericFoldOp(foldOpNames.indexOf(name));
	}
};



#endif // NUMERIC_FOLD_OP_H
