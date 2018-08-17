
#include "Exception.h"
#include <cstdio>
#include <cstdarg>


cException::cException(const char* format, ...)
{
	va_list		arglist;

	va_start(arglist, format);
	vsprintf(m_Message, format, arglist);
	va_end(arglist);
}


const char* cException::GetErrorMessage(void) const
{
	return (m_Message);
}