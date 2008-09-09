
#include "libHelpers.h"

#include "FlexiVisioncfg.h"

#include <clk.h>


// *************************************************************************

Int	hlpMsToTicks( Int ms )
{
	 return ( ms * (CLK_countspms() / CLK_getprd()) );
}

// *************************************************************************

Int hlpTicksToMs( Int ticks )
{
	//return (ticks * CLK_getprd()) / CLK_countspms();
	return (ticks  / (CLK_countspms() / CLK_getprd()));
}

// *************************************************************************

// *************************************************************************

// *************************************************************************


