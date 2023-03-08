#ifndef ITEM_ID_H
#define ITEM_ID_H

#include <cstddef>
#include <QVariant>
#include <QHashFunctions>

class ValidItemID;



class ItemID {
	bool valid;
	int id;
	
	static const int LOWEST_LEGAL_ID;
	
public:
	ItemID(int id);
	ItemID(QVariant id);
	ItemID();
	
	bool isValid() const;
	bool isNull() const;
	
	int get() const;
	QVariant asQVariant() const;
	
	ValidItemID forceValid() const;
};



class ValidItemID : public ItemID {
public:
	ValidItemID(int id);
	ValidItemID(QVariant id);
};



inline bool operator==(const ItemID& id1, const ItemID& id2)
{
	if (id1.isNull() && id2.isNull()) return true;
	if (id1.isValid() != id2.isValid()) return false;
	return id1.get() == id2.get();
}

inline bool operator!=(const ItemID& id1, const ItemID& id2)
{
	return !operator==(id1, id2);
}

inline size_t qHash(const ItemID& key, size_t seed)
{
	return qHashMulti(seed, key.isValid(), key.isValid() ? key.get() : 0);
}



#endif // ITEM_ID_H
