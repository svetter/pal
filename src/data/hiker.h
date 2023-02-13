#ifndef HIKER_H
#define HIKER_H

#include <QString>



class Hiker
{
public:
	int		hikerID;
	QString	name;
	
	Hiker(int hikerID, QString& name);
	
	bool equalTo(const Hiker* const other) const;
};



#endif // HIKER_H
