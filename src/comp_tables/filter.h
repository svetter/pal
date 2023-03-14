#ifndef FILTER_H
#define FILTER_H

#include <QVariant>
#include <QHashFunctions>

class CompositeColumn;



class Filter {
public:
	const CompositeColumn* column;
	const QVariant value;
	const bool hasSecond;
	const QVariant secondValue;
	
	inline Filter(const CompositeColumn* column, QVariant value) :
			column(column),
			value(value),
			hasSecond(false),
			secondValue(QVariant())
	{}
	
	inline Filter(const CompositeColumn* column, QVariant value, QVariant secondValue) :
			column(column),
			value(value),
			hasSecond(true),
			secondValue(secondValue)
	{}
};



inline bool operator==(const Filter& filter1, const Filter& filter2)
{
	if (filter1.column != filter2.column) return false;
	if (filter1.value != filter2.value) return false;
	if (filter1.hasSecond != filter2.hasSecond) return false;
	if (!filter1.hasSecond && !filter2.hasSecond) return true;
	return filter1.secondValue == filter2.secondValue;
}

inline bool operator!=(const Filter& filter1, const Filter& filter2)
{
	return !operator==(filter1, filter2);
}

inline size_t qHash(const Filter& key, size_t seed)
{
	return qHashMulti(seed, (void*) key.column, key.value.toByteArray(), key.hasSecond, key.secondValue.toByteArray());
}



#endif // FILTER_H
