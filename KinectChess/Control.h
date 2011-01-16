#ifndef CONTROL_H
#define CONTROL_H

#include "GLEngine.h"

#include <list>
#include <string>

using std::list;
using std::string;

struct MouseState
{
	int LeftButtonDown;
	int RightButtonDown;
	int MiddleButtonDown;
	int WheelUp;
	int WheelDown;

	int x;
	int y;

	MouseState()
	{
		LeftButtonDown = 0;
		RightButtonDown = 0;
		MiddleButtonDown = 0;
		WheelUp = 0;
		WheelDown = 0;
		
		x = 0;
		y = 0;
	}
};

class Control
{
public:
	Control(int positionX, int positionY, int width, int height);
	virtual ~Control(); //MUST be virtual otherwise not called in the correct order

	virtual bool updateControl(MouseState &state);
	virtual void drawControl(void) = 0; //opengl commands that draw controls; this line makes it an abstract class cause its 0
	virtual string getType(void) = 0;

	void setPosition(int x, int y);
	void setSize(int width, int height);
	int getWidth(void);
	int getHeight(void);

public:
	static list<Control *> controls;


protected:
	//mouse currently inside the control?
	bool		inside;
	int			posX, posY;
	int			width, height;
};

Control *addControl(Control *control);

#endif