
#include <windows.h>
#include <cstdio>
#include <cassert>


class cFontDB
{
public:
	struct Entry
	{
		// Character code
		int		code;

		// Position in the texture page
		int		x, y;

		// Width of the character, height is 32
		int		w;

		// Texture page the character is on
		int		tpage;
	};


	cFontDB(const int nb_tpages, const int nb_entries) :
		m_NbTPages(nb_tpages),
		m_NbEntries(nb_entries),
		m_CurEntry(0)
	{
		// Create the texture page data
		m_Pages = new unsigned char*[nb_tpages];
		for (int i = 0; i < nb_tpages; i++)
			m_Pages[i] = new unsigned char[256 * 256 * 3];

		// Allocate the character list
		m_Entries = new Entry[nb_entries];
	}


	~cFontDB(void)
	{
		// Delete character list
		delete [] m_Entries;

		// Delete the texture pages
		for (int i = 0; i < m_NbTPages; i++)
			delete [] m_Pages[i];
		delete [] m_Pages;
	}


	void AddEntry(const char code, const int x, const int y, const int w, const int tpage)
	{
		assert(m_CurEntry < m_NbEntries);

		// De-ref current entry
		Entry& entry = m_Entries[m_CurEntry++];

		// Fill in entry info
		entry.code = code;
		entry.x = x;
		entry.y = y;
		entry.w = w;
		entry.tpage = tpage;
	}


	unsigned char* GetTPage(const int index) const
	{
		assert(index >= 0 && index < m_NbTPages);
		return (m_Pages[index]);
	}


	void WriteToFile(const char* filename) const
	{
		int	i;

		// Open the font DB for writing
		FILE* fp = fopen(filename, "wb");
		assert(fp);

		// Write the tpage count
		fwrite(&m_NbTPages, 1, sizeof(int), fp);

		// Write each texture page
		for (i = 0; i < m_NbTPages; i++)
			fwrite(m_Pages[i], 1, 256 * 256 * 3, fp);

		// Write the entry list
		fwrite(&m_NbEntries, 1, sizeof(int), fp);
		fwrite(m_Entries, 1, m_NbEntries * sizeof(Entry), fp);

		// Shut up shop
		fclose(fp);
	}


private:
	// Number of texture pages
	int		m_NbTPages;

	// List of allocated texture pages
	unsigned char**	m_Pages;

	// Number of characters in the font
	int		m_NbEntries;

	// List of character entries
	Entry*	m_Entries;

	// Current addition
	int		m_CurEntry;
};


cFontDB* CreateFontDB(const char* name)
{
	unsigned char*	bits = 0;
	char			character = 0;

	// Allocate the font database (minus space, minus 32 control characters)
	cFontDB* font_db = new cFontDB(2, 128 - 33);

	// Create a nice device context
	HDC hdc = CreateCompatibleDC(0);

	// Create the requested font
	HFONT hfont = CreateFont(
		39,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		FALSE,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,
		DEFAULT_PITCH,
		name);

	// Create the bitmap that each character will be drawn to
	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth			= 32;
	bmi.bmiHeader.biHeight			= -32;
	bmi.bmiHeader.biPlanes			= 1;
	bmi.bmiHeader.biBitCount		= 24;
	bmi.bmiHeader.biCompression		= BI_RGB;
	HBITMAP hbitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&bits, 0, 0);

	// Set access to the bitmap through the compatible DC
	SelectObject(hdc, hbitmap);
	SelectObject(hdc, hfont);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_CENTER);
	SetTextColor(hdc, RGB(255, 255, 255));

	// Retrieve the bitmap info
	BITMAP bitmap;
	GetObject(hbitmap, sizeof(bitmap), &bitmap);

	// 2.8.8 = 128 characters (2 texture pages)
	for (int i = 0; i < 2; i++)
	{
		for (int t = 0; t < 8; t++)
		{
			for (int s = 0; s < 8; s++)
			{
				// Skip over control characters (and space)
				if (character < 33)
				{
					character++;
					continue;
				}

				// Clear the bitmap
				memset(bits, 0, 32 * 32 * 3);

				// Print the character to the bitmap
				TextOut(hdc, 32 / 2, -7, &character, sizeof(char));

				// Copy the mini-buffer into the texture page
				int y0 = t * 32;
				int x0 = s * 32;
				for (int y = y0; y < y0 + 32; y++)
					memcpy(&font_db->GetTPage(i)[y * 256 * 3 + x0 * 3], &bits[(y - y0) * 32 * 3], 32 * 3);

				// Calculate the width-reduced dimensions of the letter
				int min_x = 32, max_x = 0;
				for (int x = 0; x < 32; x++)
				{
					for (int y = 0; y < 32; y++)
					{
						// Get index in mini buffer
						int index = (y * 32 + x) * 3;

						// Get the total pixel values
						int total = bits[index + 0] + bits[index + 1] + bits[index + 2];

						// If this pixel is set, update the dimensions
						if (total)
						{
							if (x < min_x) min_x = x;
							if (x > max_x) max_x = x;
						}
					}
				}

				// Add the character to the database
				font_db->AddEntry(character, x0 + min_x - 1, y0, max_x - min_x + 2, i);

				character++;
			}
		}
	}

	FILE* fp = fopen("c:/out.raw", "wb");
	fwrite(font_db->GetTPage(1), 1, 3 * 256 * 256, fp);
	fclose(fp);

	// Release all Win32 memory
	DeleteObject(hbitmap);
	DeleteObject(hfont);
	DeleteDC(hdc);

	return (font_db);
}


int main(int argc, char* argv[])
{
	// Not enough args?
	if (argc < 3)
		return (1);

	// Do that thing that you do...
	cFontDB* font_db = CreateFontDB(argv[1]);
	font_db->WriteToFile(argv[2]);
	delete font_db;

	return (0);
}