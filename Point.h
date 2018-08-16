
#ifndef	_INCLUDED_POINT_H
#define	_INCLUDED_POINT_H


template <int N> struct Point
{
	// Array of values in all dimensions
	float	values[N];

	// Number of dimensions
	enum { n = N };

	float DistanceFrom(const Point<N>& p) const
	{
		float d = 0;
		for (int i = 0; i < N; i++)
			d += (values[i] - p.values[i]) * (values[i] - p.values[i]);
		d = (float)sqrt(d);

		return (d);
	}
};


#endif	/* _INCLUDED_POINT_H */