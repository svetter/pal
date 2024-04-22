#ifndef FOLD_OP_H
#define FOLD_OP_H

#include <QStringList>



/**
 * The different fold operations that can be performed in a FoldCompositeColumn.
 */
enum FoldOp {
	CountFold,
	AverageFold,
	SumFold,
	MaxFold,
	MinFold,
	StringListFold
};



/**
 * A struct containing the names of the fold operations and providing methods to convert between
 * names and the FoldOp enum.
 */
struct FoldOpNames
{
	/** A list of the names of the fold operations, in the order of the DataType enum. */
	inline static const QStringList foldOpNames = {
		"Count", "Average", "Sum", "Max", "Min", "StringList"
	};
	
	/**
	 * Returns the name of the given fold operation or an empty string if the type is
	 * invalid.
	 * 
	 * @param type	The fold operation.
	 */
	inline static QString getName(const FoldOp foldOp)
	{
		if (foldOp < 0 || foldOp >= foldOpNames.size()) return QString();
		return foldOpNames.at(foldOp);
	}
	
	/**
	 * Returns the fold operation corresponding to the given name or -1 if the name is invalid.
	 * 
	 * @param name	The name of the fold operation.
	 */
	inline static FoldOp getFoldOp(const QString& name)
	{
		if (!foldOpNames.contains(name)) return FoldOp(-1);
		return FoldOp(foldOpNames.indexOf(name));
	}
};



#endif // FOLD_OP_H
