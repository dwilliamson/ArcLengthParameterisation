
#ifndef	_INCLUDED_COLOURS_H
#define	_INCLUDED_COLOURS_H


static float g_Colours[][3] =
{
	{ 0, 0, 0 },
	{ 1, 1, 1 },
	{ 1, 0, 0 },
	{ 0.7f, 0.7f, 0.7f },
	{ 0.4f, 0.4f, 0.4f },
	{ 1, 1, 0 },
	{ 0, 1, 0 }
};


enum
{
	COLOUR_BLACK = 0,
	COLOUR_WHITE,
	COLOUR_RED,
	COLOUR_LGREY,
	COLOUR_DGREY,
	COLOUR_YELLOW,
	COLOUR_GREEN
};


#endif	/* _INCLUDED_COLOURS_H */