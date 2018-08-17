
#include "SDLAFont.h"
#include "SDLApp.h"			// For oGL
#include <cstdio>


cSDLAFont::cSDLAFont(const char* filename, const int window_width, const int window_height) :

	m_WindowWidth(window_width),
	m_WindowHeight(window_height)

{
	int	i;

	// Open the font database
	FILE* fp = fopen(filename, "rb");
	if (fp == 0)
		throw cException("Couldn't open file %s", filename);

	// Read the tpage count
	fread(&m_NbTPages, 1, sizeof(int), fp);

	// Generate the texture IDs
	m_Pages = new unsigned int[m_NbTPages];
	glGenTextures(m_NbTPages, m_Pages);

	// Allocate a temporary texture page in system memory
	unsigned char* buffer_ptr = new unsigned char[256 * 256 * 3];

	for (i = 0; i < m_NbTPages; i++)
	{
		// Bind to the current texture
		glBindTexture(GL_TEXTURE_2D, m_Pages[i]);

		// Read the current page from file
		fread(buffer_ptr, 1, 256 * 256 * 3, fp);

		// Upload it to video memory
		glTexImage2D(GL_TEXTURE_2D,
			0,
			3,
			256,
			256,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			buffer_ptr);

		// Setup texture states
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	// Release tpage
	delete [] buffer_ptr;

	// Allocate the character list (plus a space character)
	fread(&m_NbChars, 1, sizeof(int), fp);
	m_Chars = new Char[m_NbChars + 1];

	// Read in the character list
	fread(m_Chars, 1, m_NbChars * sizeof(Char), fp);

	// Determine the width of the as the widest character
	m_Chars[m_NbChars].w = 0;
	for (i = 0; i < m_NbChars; i++)
		if (m_Chars[i].w > m_Chars[m_NbChars].w)
			m_Chars[m_NbChars].w = m_Chars[i].w;

	m_Chars[m_NbChars].w /= 2;

	// Over it
	fclose(fp);
}


cSDLAFont::~cSDLAFont(void)
{
	delete [] m_Chars;
	glDeleteTextures(m_NbTPages, m_Pages);
	delete [] m_Pages;
}


void cSDLAFont::WriteText(const char* text_ptr, const float x, const float y, const float scale)
{
	int		i, j;
	float	xpos;

	// Table containing pointers to each character
	Char*	char_table[512];

	for (i = 0; i < 512; i++)
		char_table[i] = &m_Chars[m_NbChars];

	for (i = 0; text_ptr[i]; i++)
	{
		// Search for the current character in the font
		for (j = 0; j < m_NbChars; j++)
			if (text_ptr[i] == m_Chars[j].code)
				break;

		// Assign the table pointer
		char_table[i] = &m_Chars[j];
	}

	// Setup a flat projection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glScalef((float)m_WindowHeight / (float)m_WindowWidth, 1, 1);

	// No object-space
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Additive blending
	glBlendFunc(GL_ONE, GL_ONE);

	glColor3f(1, 1, 1);

	for (i = 0; i < m_NbTPages; i++)
	{
		// Set the texture for this page
		glBindTexture(GL_TEXTURE_2D, m_Pages[i]);

		// Start of the line
		xpos = 0;

		for (j = 0; text_ptr[j]; xpos += (char_table[j]->w + 2) * scale, j++)
		{
			// Ignore space
			if (char_table[j] == &m_Chars[m_NbChars])
				continue;

			// Ignore characters not on this texture page
			if (char_table[j]->tpage != i)
				continue;

			// Figure out quad dimensions
			float x0 = x + xpos / (float)m_WindowWidth;
			float x1 = x0 + ((float)char_table[j]->w / (float)m_WindowWidth) * scale;
			float y0 = y;
			float y1 = y0 + (31.0f / (float)m_WindowWidth) * scale;

			// Figure out texture dimensions
			float u0 = (float)char_table[j]->x / 255.0f;
			float u1 = u0 + (float)char_table[j]->w / 255.0f;
			float v1 = (float)char_table[j]->y / 255.0f;
			float v0 = v1 + 31.0f / 255.0f;

			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(u0, v0);
			glVertex3f(x0, y0, 1);
			glTexCoord2f(u0, v1);
			glVertex3f(x0, y1, 1);
			glTexCoord2f(u1, v0);
			glVertex3f(x1, y0, 1);
			glTexCoord2f(u1, v1);
			glVertex3f(x1, y1, 1);
			glEnd();
		}
	}

	// Restore old matrices
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Take a guess at what the user's old blending mode was (no blending)
	glBlendFunc(GL_ONE, GL_ZERO);
	glBindTexture(GL_TEXTURE_2D, 0);
}


float cSDLAFont::GetLineHeight(void) const
{
	return (32.0f / m_WindowHeight);
}
