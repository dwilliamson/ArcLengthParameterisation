
#ifndef	_INCLUDED_EXCEPTION_H
#define	_INCLUDED_EXCEPTION_H


class cException
{
public:
	// Constructor with arguments
	cException(const char* format, ...);

	// Return message array
	const char*	GetErrorMessage(void) const;

private:
	// Error message
	char	m_Message[512];
};


#endif	/* _INCLUDED_EXCEPTION_H */