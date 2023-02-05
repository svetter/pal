#ifndef HIKER_H
#define HIKER_H

#include <QString>



class Hiker
{
public:
	int		hikerID;
	QString	name;
	
	Hiker(int hikerID, QString& name);
	
	bool equalTo(Hiker* other);
};



#endif // HIKER_H
