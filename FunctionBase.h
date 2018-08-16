
#ifndef	_INCLUDED_FUNCTIONBASE_H
#define	_INCLUDED_FUNCTIONBASE_H


#ifndef	_INCLUDED_POINT_H
	#include "Point.h"
#endif


template <int N> struct FunctionBase
{
	// Virtual dtor for deletes
	virtual ~FunctionBase(void) { }

	// Evaluation function
	virtual Point<N> P(const float u) const = 0;

	// Analytically compute length of the curve
	virtual float L(const float u0, const float u1) const = 0;
};


#endif	/* _INCLUDED_FUNCTIONBASE_H */