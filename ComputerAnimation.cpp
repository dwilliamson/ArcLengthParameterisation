
#include "ComputerAnimation.h"
#include "Colours.h"
#include "Function.h"
#include "SDLAFont.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstdarg>


static const float PI = 3.1415926f;


namespace
{
	template <typename T> const T& min(const T& a, const T& b)
	{
		return (a < b ? a : b);
	}


	struct CubicPolynomial
	{
		float	a, b, c, d;

		float P(const float u) const
		{
			float u2 = u * u;
			float u3 = u2 * u;
			return (a * u3 + b * u2 + c * u + d);
		}

		
		float D1(const float u) const
		{
			// First differential function
			float _a = 3 * a;
			float _b = 2 * b;

			// Evaluate it
			return (_a * u * u + _b * u + c);
		}


		float D2(const float u) const
		{
			// Second differential function
			float _a = 3 * 2 * a;
			float _b = 2 * b;

			// Evaluate it
			return (_a * u + _b);
		}
	};


	inline float Random(void)
	{
		return (rand() / (float)RAND_MAX);
	}


	inline float EaseSine(const float t)
	{
		return (((float)sin(t * PI - PI / 2) + 1) / 2);
	}


	inline float EaseSineSegments(const float t, const float k1, const float k2)
	{
		float s;

		const float f = k1 * 2 / PI + k2 - k1 + (1.0f - k2) * 2 / PI;

		if (t < k1)
			s = k1 * (2 / PI) * ((float)sin((t / k1) * PI / 2 - PI / 2) + 1);
		else if (t < k2)
			s = 2 * k1 / PI + t - k1;
		else
			s = 2 * k1 / PI + k2 - k1 + ((1 - k2) * (2 / PI)) * (float)sin(((t - k2) / (1 - k2)) * PI / 2);

		return (s / f);
	}
};


cComputerAnimation::cComputerAnimation(const int width, const int height) :

	cSDLApp(width, height, true),
	m_S(0),
	m_U(0)

{
	m_Function = new tFunction<2, CubicPolynomial>(20);

	srand(time(0));
	Regenerate();
}


cComputerAnimation::~cComputerAnimation(void)
{
	delete m_Font;
	delete m_Function;
}


bool cComputerAnimation::ProcessFrame(void)
{
	// Clear screen
	glClear(GL_COLOR_BUFFER_BIT);

	// No textures for drawing the remaining stuff
	glBindTexture(GL_TEXTURE_2D, 0);

	DrawGrid(COLOUR_DGREY);
	DrawCurve();

	// Get the cast function
	tFunction<2, CubicPolynomial>* f_ptr = static_cast<tFunction<2, CubicPolynomial>*>(m_Function);

	// Increment arc-length linearly
	m_S = m_S + 0.01f;
	if (m_S >= f_ptr->L(0, 1)) m_S = 0;

	// Get the parameter value at the current arc-length
	m_U = f_ptr->GetParameterNewtonRaphson(m_S);

	Point<2> p = m_Function->P(m_U);
	DrawRing(p.values[0], p.values[1], 0.02f, 0.1f, COLOUR_WHITE);

	if (m_KeysReleased[SDLK_SPACE])
		Regenerate();

	int i;
	for (i = 0; i < min(21, f_ptr->nb_entries); i++)
		WriteText(-1.25f, i, "%d: %.2f -> %.4f", i, f_ptr->arc_lengths[i * 2 + 0], f_ptr->arc_lengths[i * 2 + 1]);
	WriteText(-1.25f, i, "(nb_entries = %d)", f_ptr->nb_entries);

	WriteText(0.2f, 0, "Arc-length (Nearest): %f", f_ptr->GetArcLengthNearestAdaptive(m_U));
	WriteText(0.2f, 1, "Arc-length (Lerped): %f", f_ptr->GetArcLengthLerpedAdaptive(m_U));
	WriteText(0.2f, 2, "Parameter (Nearest): %f", f_ptr->GetParameterNearest(m_S));
	WriteText(0.2f, 3, "Parameter (Lerped): %f", m_U);
	WriteText(0.2f, 4, "Trapezoid (Error): %f", f_ptr->IntegrateTrapezoidError(0, m_U, 5));
	WriteText(0.2f, 5, "Trapezoid (Fixed): %f", f_ptr->IntegrateTrapezoidFixed(0, m_U, 10));
	WriteText(0.2f, 6, "Simpson (Error): %f", f_ptr->IntegrateSimpsonError(0, m_U, 5));
	WriteText(0.2f, 7, "Romberg: %f", f_ptr->IntegrateRomberg(0, m_U));
	WriteText(0.2f, 8, "Gaussian Quadrature: %f", f_ptr->GaussianQuadrature(0, m_U));
	WriteText(0.2f, 9, "Adaptive Gaussian Quadrature: %f", f_ptr->GetArcLengthAdaptiveGaussian(m_U));
	WriteText(0.2f, 10, "Newton-Raphson Parameter: %f", f_ptr->GetParameterNewtonRaphson(m_S));

	Point<2> p0 = m_Function->P(f_ptr->GetParameterNewtonRaphson(m_S - 0.01f));
	WriteText(0.2f, 25, "Speed: %f", p0.DistanceFrom(p));

	float l = f_ptr->L(0, 1);
	float s = EaseSine(m_S / l) * l;
	float u = f_ptr->GetParameterNewtonRaphson(s);
	Point<2> pr = m_Function->P(u);
	DrawRing(pr.values[0], pr.values[1], 0.02f, 0.07f, COLOUR_RED);

	s = EaseSineSegments(m_S / l, 0.2f, 0.8f) * l;
	u = f_ptr->GetParameterNewtonRaphson(s);
	Point<2> pg = m_Function->P(u);
	DrawRing(pg.values[0], pg.values[1], 0.02f, 0.05f, COLOUR_GREEN);

	return (true);
}


void cComputerAnimation::BeforeSwitch(void)
{
	delete m_Font;
}


void cComputerAnimation::AfterSwitch(void)
{
	glDisable(GL_DEPTH_TEST);

	// Standard SDL clear colour
	glClearColor(0, 0, 0, 1);

	m_Font = CreateFont("ArialItalic.fdb");
}


void cComputerAnimation::Regenerate(void)
{
	// Cast to function type
	tFunction<2, CubicPolynomial>* f_ptr = static_cast<tFunction<2, CubicPolynomial>*>(m_Function);

	// Generate random cubic co-efficients
	for (int i = 0; i < 2; i++)
	{
		f_ptr->curve[i].a = Random() - 0.5f;
		f_ptr->curve[i].b = Random() - 0.5f;
		f_ptr->curve[i].c = Random() - 0.5f;
		f_ptr->curve[i].d = Random() - 0.5f;
	}

	// Basic non-adaptive initialisation
	//f_ptr->InitTable();

	// Chance of large under-sampling with linear distance based method so bring in
	// quite a small maximum distance of subdivision
	//f_ptr->InitTableAdaptive(0.00001f, 0.05f);

	// No danger of under-sampling with newton-raphson
	f_ptr->InitTableAdaptiveGaussian(1e-6f, 0.5f);

	m_S = 0;
}


float cComputerAnimation::XPos(const int x) const
{
	return (((float)x / (float)m_Width) * 2 - 1);
}


float cComputerAnimation::YPos(const int y) const
{
	return (((float)y / (float)m_Height) * 2 - 1);
}


void cComputerAnimation::DrawCircle(const float x, const float y, const float radius, const int colour)
{
	// Set colour
	glColor3fv(g_Colours[colour]);

	// Position the circle
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(x, y, 1);

	// Subdivision parameters
	static const int NB_SUBS = 32;
	static const float INCREMENT = 6.2824f / (NB_SUBS - 1);

	// Begin circle triangle fan
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, 0);

	for (int i = 0; i < NB_SUBS; i++)
	{
		// Increment over the circumference
		float t = INCREMENT * i;
		float x = (float)cos(t) * radius;
		float y = (float)sin(t) * radius;

		// Plot the point
		glVertex3f(x, y, 0);
	}

	glEnd();
}


void cComputerAnimation::DrawRing(const float x, const float y, const float inner_radius, const float outer_radius, const int colour)
{
	// Set colour
	glColor3fv(g_Colours[colour]);

	// Position the ring
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(x, y, 1);

	// Subdivision parameters
	static const int NB_SUBS = 32;
	static const float INCREMENT = 6.2824f / (NB_SUBS - 1);

	// Begin ring triangle strip
	glBegin(GL_TRIANGLE_STRIP);

	for (int i = 0; i < NB_SUBS; i++)
	{
		// Increment around circle
		float t = INCREMENT * i;

		// Get inner point
		float x0 = (float)cos(t) * inner_radius;
		float y0 = (float)sin(t) * inner_radius;

		// Get outer point
		float x1 = (float)cos(t) * outer_radius;
		float y1 = (float)sin(t) * outer_radius;

		// Build the strip
		glVertex3f(x1, y1, 0);
		glVertex3f(x0, y0, 0);
	}

	glEnd();
}


void cComputerAnimation::DrawCurve(void)
{
	// Draw the start and end points
	Point<2> a = m_Function->P(0);
	Point<2> b = m_Function->P(1);
	DrawCircle(a.values[0], a.values[1], 0.02f, COLOUR_YELLOW);
	DrawCircle(b.values[0], b.values[1], 0.02f, COLOUR_YELLOW);

	// Position in world space
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw a curve connecting the end points
	glBegin(GL_LINES);
	for (float i = 0; i < 1; i += 0.01f)
	{
		Point<2> a = m_Function->P(i);
		Point<2> b = m_Function->P(i + 0.01f);
		glVertex3f(a.values[0], a.values[1], 0);
		glVertex3f(b.values[0], b.values[1], 0);
	}
	glEnd();
}


void cComputerAnimation::DrawGrid(const int colour)
{
	// Set colour
	glColor3fv(g_Colours[colour]);

	// Position in world space
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_LINES);
	for (float i = -1.5f; i < 1.5f; i += 0.1f)
	{
		glVertex3f(i, 1, 0);
		glVertex3f(i, -1, 0);
	}
	for (float j = -1; j < 1; j += 0.1f)
	{
		glVertex3f(-1.5f, j, 0);
		glVertex3f(1.5f, j, 0);
	}
	glEnd();
}


void cComputerAnimation::WriteText(const float x, const int y, const char* format, ...)
{
	va_list	arglist;
	char	buffer[512];

	// Build the formatted string
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);

	// Write the text
	m_Font->WriteText(buffer, x, 0.8f - m_Font->GetLineHeight() * y, 1);
}