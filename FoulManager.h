#ifndef _FOULMANAGER_
#define _FOULMANAGER_

class FoulManager
{
	void isHandBallInHole(); 
	bool isEightBallBadToIn(); 
	bool isEightBallWithFoul(); 

public:
	FoulManager();
	void reset(); 
	void checkFoul();
	bool isLose(); 
};

#endif