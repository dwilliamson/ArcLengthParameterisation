
#ifndef	_INCLUDED_SDLAFONT_H
#define	_INCLUDED_SDLAFONT_H


class cSDLAFont
{
public:
	cSDLAFont(const char* filename, const int window_width, const int window_height);
	~cSDLAFont(void);

	void	WriteText(const char* text_ptr, const float x, const float y, const float scale);

	float	GetLineHeight(void) const;

private:
	struct Char
	{
		// ASCII code
		int		code;

		// Position of this character in the tpage
		int		x, y;

		// Width of the character
		int		w;

		// Texture page the character is on
		int		tpage;
	};

	// Number of texture pages used for this font
	int		m_NbTPages;

	// OpenGL texture ID per page
	unsigned int*	m_Pages;

	// List of characters supported by the font
	int		m_NbChars;
	Char*	m_Chars;

	// Dimensions of the window the text is being drawn in
	int		m_WindowWidth;
	int		m_WindowHeight;
};


#endif	/* _INCLUDED_SDLAFONT_H */