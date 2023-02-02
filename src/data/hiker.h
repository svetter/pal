#ifndef HIKER_H
#define HIKER_H

#include <QString>



class Hiker
{
public:
	int		hikerID;
	QString	name;
	
	bool equalTo(Hiker* other) {
		if (name != other->name)	return false;
		return true;
	}
};



#endif // HIKER_H
