
#include "libDebug.h"
#include "classControl.h"
#include "stdio.h"

void dbgAssertLog(Bool b, char * file, Int line)
{
	if (!b)
	{
		CControl::PostDebugMsg( "ASSERT failed in file %s:%d", file, line );
		while(1)
		{
			//CControl::PostDebugMsg(str, "assert: file:%s, line:%d", file, line);
			TSK_sleep(10000);
		}
	}	
}

void dbgLog( const char * strFormat, ... )
{
	char str[256];
	
	// Format message to the string
	va_list ap;
	va_start(ap, strFormat);
	vsprintf(str, strFormat, ap);
  	va_end(ap);
  	
  	CControl::PostDebugMsg( str );
}
