
#include "classBusCoupler.h"
#include "libDebug.h"

#include <string.h>


CBusCoupler::CBusCoupler( )
{
}

// *************************************************************************

void CBusCoupler::Config( PPUSER_DevHandle hSerial, Int nBCAddr, Int nNumInputs, Int nNumOutputs )
{
	m_hSerial = hSerial;
	
	m_nBCAddr = nBCAddr;	
	m_nNumInputs = nNumInputs;
	m_nNumOutputs = nNumOutputs;
	
	PrepareImages();
}

// *************************************************************************

