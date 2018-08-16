
#ifndef	_INCLUDED_COMPUTERANIMATION_H
#define	_INCLUDED_COMPUTERANIMATION_H


#ifndef	_INCLUDED_SDLAPP_H
	#include <SDLApp.h>
#endif

#ifndef	_INCLUDED_FUNCTIONBASE_H
	#include "FunctionBase.h"
#endif


class cComputerAnimation : public cSDLApp
{
public:
	// Constructor/destructor
	cComputerAnimation(const int width, const int height);
	~cComputerAnimation(void);

private:
	bool	ProcessFrame(void);
	void	BeforeSwitch(void);
	void	AfterSwitch(void);

	float	XPos(const int x) const;
	float	YPos(const int y) const;
	void	DrawCircle(const float x, const float y, const float radius, const int colour);
	void	DrawRing(const float x, const float y, const float inner_radius, const float outer_radius, const int colour);
	void	DrawCurve(void);
	void	DrawGrid(const int colour);

	void	Regenerate(void);

	void	WriteText(const float x, const int y, const char* format, ...);

	FunctionBase<2>* m_Function;

	cSDLAFont*	m_Font;

	float	m_U;
	float	m_S;
};


#endif	/* _INCLUDED_COMPUTERANIMATION_H */