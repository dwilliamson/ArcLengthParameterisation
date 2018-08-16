
#ifndef	_INCLUDED_FUNCTION_H
#define	_INCLUDED_FUNCTION_H


#ifndef	_INCLUDED_FUNCTIONBASE_H
	#include "FunctionBase.h"
#endif


template <int N, typename T> struct tFunction : public FunctionBase<N>
{
	tFunction(const int _nb_entries) : nb_entries(_nb_entries + 1)
	{
		// Allocate parameter/arc-length pairs
		arc_lengths = new float[nb_entries * 2];

		// Calculate the parameter distance between entries
		entry_distance = 1.0f / (float)nb_entries;
	}


	~tFunction(void)
	{
		// Release all memory
		delete [] arc_lengths;
	}


	// Dimension count
	enum { n = N };


	Point<N> P(const float u) const
	{
		Point<N> p;

		// For each dimension
		for (int i = 0; i < N; i++)
			p.values[i] = curve[i].P(u);

		return (p);
	}


	void InitTable(void)
	{
		// The first two entries are zero
		int i = 2;
		arc_lengths[0] = 0;
		arc_lengths[1] = 0;

		for (float u = entry_distance; i < nb_entries * 2; u += entry_distance, i += 2)
		{
			// Sample previous point and this point along curve
			Point<N> p0 = P(u - entry_distance);
			Point<N> p1 = P(u);

			// Fill in the table entries
			arc_lengths[i + 0] = u;
			arc_lengths[i + 1] = arc_lengths[i - 1] + p0.DistanceFrom(p1);
		}
	}


	struct Pair
	{
		// Parametric value
		float	u;

		// Arc-length value
		float	s;

		// Next pair in the linked list
		Pair*	next;


		static void Add(Pair *& first, const float u, const float s)
		{
			// Create the new pair
			Pair* add = new Pair;
			add->u = u;
			add->s = s;
			add->next = 0;

			// Search up through the current list
			Pair* cur = 0, *last = 0;
			for (cur = first; cur; cur = cur->next)
			{
				// Insert before?
				if (add->u <= cur->u)
					break;

				last = cur;
			}

			// Insert in the linked list
			add->next = cur;
			last->next = add;
		}
	};


	void InitTableAdaptive(const float tolerance, const float max_dist)
	{
		struct Segment
		{
			static void Process(tFunction<N, T>* f_ptr, const float min_u, const float max_u, const float max_dist, const float tolerance, Pair* pairs)
			{
				// Get parameteric values for the endpoints and midpoint
				float u[3] =
				{
					min_u,
					(min_u + max_u) / 2,
					max_u
				};

				// Sample the all three points along the segment
				Point<N> p[3];
				p[0] = f_ptr->P(u[0]);
				p[1] = f_ptr->P(u[1]);
				p[2] = f_ptr->P(u[2]);

				// Estimate the lengths of the two halves and the entire segment
				float l[3] =
				{
					p[1].DistanceFrom(p[0]),
					p[2].DistanceFrom(p[1]),
					p[2].DistanceFrom(p[0])
				};

				// Too much error?
				if (u[2] - u[1] >= max_dist || fabs(l[0] + l[1] - l[2]) > tolerance)
				{
					// Further split the two halves of this segment
					Process(f_ptr, u[0], u[1], max_dist, tolerance, pairs);
					Process(f_ptr, u[1], u[2], max_dist, tolerance, pairs);
				}

				// Just fine...
				else
				{
					// Add the midpoint
					tFunction<N, T>::Pair::Add(pairs, u[1], l[0]);
					tFunction<N, T>::Pair::Add(pairs, u[2], l[1]);
				}
			}
		};

		// Create the top of the linked pair list, contains the <0, 0> entry
		Pair* pairs = new Pair;
		pairs->u = 0;
		pairs->s = 0;
		pairs->next = 0;

		Segment::Process(this, 0, 1, max_dist, tolerance, pairs);

		// Count the number of entries in the table
		nb_entries = 0;
		Pair* cur;
		for (cur = pairs; cur; cur = cur->next)
			nb_entries++;

		// Allocate the table
		arc_lengths = new float[2 * nb_entries];

		// Fill in the start entry
		arc_lengths[0] = 0;
		arc_lengths[1] = 0;

		// Copy all entries after the first, summing the arc-lengths along the way
		nb_entries = 1;
		for (cur = pairs->next; cur; cur = cur->next)
		{
			arc_lengths[nb_entries * 2 + 0] = cur->u;
			arc_lengths[nb_entries * 2 + 1] = cur->s + arc_lengths[nb_entries * 2 - 1];
			nb_entries++;
		}
	}


	int BinarySearch(const float v, const int offset) const
	{
		int min_i = 0, max_i = nb_entries - 1;
		int mid_point;

		// Get table direction
		bool is_ascending = (arc_lengths[max_i * 2 + offset] >= arc_lengths[1]);

		while (max_i - min_i > 1)
		{
			// Compute the midpoint
			mid_point = (min_i + max_i) >> 1;

			// Replace upper or lower limits based on direction of the answer
			if (v >= arc_lengths[mid_point * 2 + offset] == is_ascending)
				min_i = mid_point;
			else
				max_i = mid_point;
		}

		return (min_i);
	}


	float GetArcLengthNearest(const float u) const
	{
		return (arc_lengths[(int)(u / entry_distance + 0.5f) * 2 + 1]);
	}


	float GetArcLengthNearestAdaptive(const float u) const
	{
		return (arc_lengths[BinarySearch(u, 0) * 2 + 1]);
	}


	float GetArcLengthLerpedI(const int i, const float u) const
	{
		// Get entries on either side of the input
		float v0 = arc_lengths[i * 2];
		float v1 = arc_lengths[i * 2 + 2];

		// Get arc-lengths on either side of the input
		float l0 = arc_lengths[i * 2 + 1];
		float l1 = arc_lengths[i * 2 + 3];

		// Calculate interpolation parameter
		float t = (u - v0) / (v1 - v0);

		// Now lerp between the arc-lengths
		return (l0 + t * (l1 - l0));
	}


	float GetArcLengthLerped(const float u) const
	{
		// Calculate nearest entry less than given value
		int i = (int)(u / entry_distance);

		return (GetArcLengthLerpedI(i, u));
	}


	float GetArcLengthLerpedAdaptive(const float u) const
	{
		int i = BinarySearch(u, 0);

		return (GetArcLengthLerpedI(i, u));
	}


	float GetParameterNearest(const float arc_length) const
	{
		// Search for the closest matching arc-length
		int i = BinarySearch(arc_length, 1);

		return (arc_lengths[i * 2 + 0]);
	}


	float GetParameterLerped(const float arc_length) const
	{
		// Search for the closest matching arc-length
		int i = BinarySearch(arc_length, 1);

		// Get entries on either side of the found
		float v0 = arc_lengths[i * 2];
		float v1 = arc_lengths[i * 2 + 2];

		// Get arc-lengths on either side
		float l0 = arc_lengths[i * 2 + 1];
		float l1 = arc_lengths[i * 2 + 3];

		// Calculate interpolation parameter
		float t = (arc_length - l0) / (l1 - l0);

		// Now lerp between the parameters
		return (v0 + t * (v1 - v0));
	}


	float L(const float u0, const float u1) const
	{
		return (GetArcLengthLerpedAdaptive(u1) - GetArcLengthLerpedAdaptive(u0));
	}


	// This samples the arc-length integral function: modulus[dP/du]
	float EvalIntFunc(const float u) const
	{
		float val = 0;

		// Sum the squared first differentials for each dimension at the given point
		for (int i = 0; i < N; i++)
		{
			float d = curve[i].D1(u);
			val += d * d;
		}

		// This calculates the modulus to complete the function evaluation
		return ((float)sqrt(val));
	}


	// Rather than integrating over the required n samples in one linear sweep, subsequent
	// calls to this method will refine previous calls by subdividing the sample points.
	// This allows the method to be used adaptively until the error is limited to within
	// a certain tolerance.
	// Because of this, n is not the number of samples to take, but 2^(n-2) are the number
	// of sample points to add to the current approximation.
	float IntegrateTrapezoid(const float u0, const float u1, const int n) const
	{
		// First evaluation
		if (n == 0)
			last_eval = 0.5f * (u1 - u0) * (EvalIntFunc(u0) + EvalIntFunc(u1));

		else
		{
			int		nb_samples = 1, i;
			float	sum = 0;

			// Calculate the number of samples to take
			nb_samples = (nb_samples << (n - 2));

			// Spacing between samples
			float h = (u1 - u0) / (float)nb_samples;

			// Start off in-between the previous sample points
			float x = u0 + 0.5f * h;

			// Sum all function evaluations
			for (i = 0; i < nb_samples; i++)
			{
				sum += EvalIntFunc(x);
				x += h;
			}

			// Refine the approximation
			last_eval = 0.5f * (last_eval + (u1 - u0) * sum / (float)nb_samples);
		}

		return (last_eval);
	}


	// Use extended trapezoidal rule to a fixed number of sample points
	// Number of sample points used = 2^n - 1
	float IntegrateTrapezoidFixed(const float u0, const float u1, const int n) const
	{
		// Refine to the desired amount
		for (int i = 0; i < n + 1; i++)
			IntegrateTrapezoid(u0, u1, i);

		return (last_eval);
	}


	float IntegrateTrapezoidError(const float u0, const float u1, const int min_n) const
	{
		int		i;
		
		// Avoid early convergence
		for (i = 0; i < min_n; i++)
			IntegrateTrapezoid(u0, u1, i);

		// Evaluation at previous iteration
		float prev_eval = last_eval;

		for ( ; i < 10; i++)
		{
			// Refine evaluation
			IntegrateTrapezoid(u0, u1, i);

			// If the error has been limited enough, return the value
			if (fabs(last_eval - prev_eval) < 1e-5f * fabs(prev_eval) ||
				(last_eval == 0 && prev_eval == 0))
				return (last_eval);

			prev_eval = last_eval;
		}

		// As close as we can get...
		return (last_eval);
	}


	float IntegrateSimpsonError(const float u0, const float u1, const int min_n) const
	{
		int		i;
		float	s;

		// Avoid early convergence
		for (i = 0; i < min_n; i++)
			IntegrateTrapezoid(u0, u1, i);

		// Evaluation at previous iteration
		float prev_eval = last_eval;
		float prev_s = -1e5;

		for ( ; i < 10; i++)
		{
			// Refine evaluation
			IntegrateTrapezoid(u0, u1, i);

			s = (4 * last_eval - prev_eval) / 3.0f;

			// If the error has been limited enough, return the value
			if (fabs(s - prev_s) < 1e-5f * fabs(prev_s) ||
				(s == 0 && prev_s == 0))
				return (s);

			prev_s = s;
			prev_eval = last_eval;
		}

		return (s);
	}


	// Given an array of (x,y) values, this method evaluates y at any given x by constructing
	// the unique interpolating polynomial of degree n-1 and evaluating it at x. Can be used
	// for interpolation or extrapolation.
	float LagrangePolynomialInterpolation(const float* xa, const float* ya, const float x, const int n) const
	{
		float	y = 0;
		int		i;

		for (int j = 0; j < n; j++)
		{
			// Calculate numerator
			float num = 1;
			for (i = 0; i < n; i++)
				if (i != j)
					num *= (x - xa[i]);

			// Calculate denominator
			float den = 1;
			for (i = 0; i < n; i++)
				if (i != j)
					den *= (xa[j] - xa[i]);

			// Sum the evaluation
			y += (num / den) * ya[j];
		}

		return (y);
	}


	float NevillePolynomialInterpolation(const float* xa, const float* ya, const float x, const int n, float& error_y) const
	{
		int		i, j, closest_entry = 0;

		// Yucky (for now)... allocate the tableau
		float* C = new float[n];
		float* D = new float[n];

		// Closest entry is set as the first
		float diff = (float)fabs(x - xa[0]);

		for (i = 0; i < n; i++)
		{
			// Update if this point is the closest to the given x
			float diff_t = (float)fabs(x - xa[i]);
			if (diff_t < diff)
			{
				closest_entry = i;
				diff = diff_t;
			}

			// Initialise tableaus
			C[i] = ya[i];
			D[i] = ya[i];
		}

		// Initial approximation is the closest entry
		float y = ya[closest_entry--];

		for (j = 0; j < n - 1; j++)
		{
			for (i = 0; i < n - j - 1; i++)
			{
				// Get distance from given points (assumes monotically increasing)
				float ho = xa[i] - x;
				float hp = xa[i + j + 1] - x;

				// Check for singular denominator (only if points are equal on the x)
				float den = ho - hp;
				if (den == 0)
					throw cException("NevillePolynomialInterpolation: Singular denominator");

				// Update the tableaus
				den = (C[i + 1] - D[i]) / den;
				C[i] = ho * den;
				D[i] = hp * den;
			}

			// After each column in the tableau is completed we decide which correction, C or D,
			// we want to add to our accumulating value of y, i.e. which path to take through the
			// tableau - forking up or down. We do this in such a way as to take the most "straight
			// line" route through the tableau to its apex, updating the closest entry accordingly
			// to keep track of where we are. This route keeps the partial approximations
			// centered on the target x. The last dy added is thus the error indication.

			if (2 * closest_entry < n - j - 1)
				error_y = C[closest_entry + 1];
			else
				error_y = D[closest_entry--];

			y += error_y;
		}

		delete [] D;
		delete [] C;

		return (y);
	}


	float IntegrateRomberg(const float u0, const float u1)
	{
		static const int	MAX_NB_EVALS = 10;
		static const int	K = 5;				// How many successive trapezoid results to
												//  extrapolate from

		float	s[MAX_NB_EVALS];		// Successive trapezoidal approximations...
		float	h[MAX_NB_EVALS + 1];	// ...and their stepsizes

		int		i;
		float	y, error_y;

		h[0] = 1;

		// Avoid early convergence
		for (i = 0; i < K; i++)
		{
			s[i] = IntegrateTrapezoid(u0, u1, i);
			h[i + 1] = 0.25f * h[i];
		}

		for ( ; i < MAX_NB_EVALS; i++)
		{
			// Refine evaluation
			s[i] = IntegrateTrapezoid(u0, u1, i);

			// Extrapolate the last K results to h=0
			// NOTE: Can use Lagrange algorithm here (slower, less elegant)
			y = NevillePolynomialInterpolation(&h[i - K], &s[i - K], 0, K, error_y);

			// Break out with limited error
			if ((float)fabs(error_y) <= 1e-6f * (float)fabs(y))
				return (y);

			// Update the step-size.
			h[i + 1] = 0.25f * h[i];
		}

		return (y);
	}


	float GaussianQuadrature(const float u0, const float u1) const
	{
		// Table of abscissas (N = 10, weighted around midpoint, so only 5)
		static float x[5] =
		{
			0.1488743389f,
			0.4333953941f,
			0.6794095682f,
			0.8650633666f,
			0.9739065285f
		};

		// Table of weights per sample
		static float w[5] =
		{
			0.2955242247f,
			0.2692667193f,
			0.2199863625f,
			0.1494513491f,
			0.0666713443f
		};

		// Calculate parametric mid-point and range either side of it
		float mid_point = 0.5f * (u0 + u1);
		float range = mid_point - u0;

		// Will be twice the average value of the function since the ten weights
		// (5 weights used twice) sum to 2.
		float s = 0;

		for (int i = 0; i < 5; i++)
		{
			// Get sample point either side of the mid-point
			float dx = range * x[i];

			// Sum the weighted sample values
			s += w[i] * (EvalIntFunc(mid_point + dx) + EvalIntFunc(mid_point - dx));
		}

		// Scale result to integration range
		return (s * range);
	}


	void InitTableAdaptiveGaussian(const float tolerance, const float max_dist)
	{
		struct Segment
		{
			static void Process(tFunction<N, T>* f_ptr, const float min_u, const float max_u, const float max_dist, const float tolerance, Pair* pairs)
			{
				// Get parameteric values for the endpoints and midpoint
				float u[3] =
				{
					min_u,
					(min_u + max_u) / 2,
					max_u
				};

				// Use gaussian quadrature to approximate the two halves and the entire segment
				float l[3] =
				{
					f_ptr->GaussianQuadrature(u[0], u[1]),
					f_ptr->GaussianQuadrature(u[1], u[2]),
					f_ptr->GaussianQuadrature(u[0], u[2])
				};

				// Too much error?
				if (u[2] - u[1] >= max_dist || fabs(l[0] + l[1] - l[2]) > tolerance)
				{
					// Further split the two halves of this segment
					Process(f_ptr, u[0], u[1], max_dist, tolerance, pairs);
					Process(f_ptr, u[1], u[2], max_dist, tolerance, pairs);
				}

				// Just fine...
				else
				{
					// Add the midpoint
					tFunction<N, T>::Pair::Add(pairs, u[1], l[0]);
					tFunction<N, T>::Pair::Add(pairs, u[2], l[1]);
				}
			}
		};

		// Create the top of the linked pair list, contains the <0, 0> entry
		Pair* pairs = new Pair;
		pairs->u = 0;
		pairs->s = 0;
		pairs->next = 0;

		Segment::Process(this, 0, 1, max_dist, tolerance, pairs);

		// Count the number of entries in the table
		nb_entries = 0;
		Pair* cur;
		for (cur = pairs; cur; cur = cur->next)
			nb_entries++;

		// Allocate the table
		arc_lengths = new float[2 * nb_entries];

		// Fill in the start entry
		arc_lengths[0] = 0;
		arc_lengths[1] = 0;

		// Copy all entries after the first, summing the arc-lengths along the way
		nb_entries = 1;
		for (cur = pairs->next; cur; cur = cur->next)
		{
			arc_lengths[nb_entries * 2 + 0] = cur->u;
			arc_lengths[nb_entries * 2 + 1] = cur->s + arc_lengths[nb_entries * 2 - 1];
			nb_entries++;
		}
	}


	float GetArcLengthAdaptiveGaussian(const float u) const
	{
		// Do a binary search for the nearest parameter
		int	i = BinarySearch(u, 0);

		// Refinement is now good enough to have an accurate approximation with
		// gaussian quadrature
		return (arc_lengths[i * 2 + 1] + GaussianQuadrature(arc_lengths[i * 2 + 0], u));
	}


	float GetParameterNewtonRaphson(const float s) const
	{
		// Search for the arc-lengths closest to the requested one
		int index = BinarySearch(s, 1);

		// Get parameters on either side of the arc-length
		float v0 = arc_lengths[index * 2];
		float v1 = arc_lengths[index * 2 + 2];

		// Get arc-lengths on either side
		float l0 = arc_lengths[index * 2 + 1];
		float l1 = arc_lengths[index * 2 + 3];

		// Calculate interpolation parameter
		float t = (s - l0) / (l1 - l0);

		// Initial guess is a lerp between the parameters
		float p = v0 + t * (v1 - v0);

		// The power of Newton-Raphson lies in it's very quick local convergence.
		// Using the table-based approach to locate a very close initial guess at the
		// root, we can use a minimal number of iterations to quickly converge on a
		// more accurate root.
		for (int i = 0; i < 2; i++)
		{
			// Numerator function
			float f = s - l0 - GaussianQuadrature(v0, p);

			// Denominator derivative
			float fd = -EvalIntFunc(p);

			// Single iteration
			p = p - f / fd;
		}

		return (p);
	}


	T curve[N];

	int		nb_entries;

	float*	arc_lengths;

	float	entry_distance;

	mutable float	last_eval;
};


#endif	/* _INCLUDED_FUNCTION_H */