
#include "classVisFourierClassifier.h"
#include "classVisTracker.h"


#define FP16_MULT(a,b, fract)	( ( (Int32)(a) * (Int32)(b) + (1 << (fract-1) ) ) >> (fract))


	
// *************************************************************************

CVisFourierClassifier::CVisFourierClassifier( const Char * strName )
		:	CVisComponent( strName, "FourierClass" ),
#ifdef _DEBUG
			m_oportDifference( "fftDifference", CVisPort::PDT_DATA, CVisOutputPort::OUTPORT_FAST ),
#endif
			m_iportFourierData( "dataFourier", CVisPort::PDT_DATA ),
			m_iportObjectsList( "objPotatoes", CVisPort::PDT_DATA )
			
{

	m_iportFourierData.Init( this );
	m_iportObjectsList.Init( this );

#ifdef _DEBUG
	m_oportDifference.Init( this );
	m_oportDifference.SetBufferSize( NUM_COEFFS * sizeof( Uint32 ) );
#endif

	

	// Acquire memory for the coefficient variance and mean value accumulators
	CVisBufferManager::Instance()->RequestBuffer( this, NUM_COEFFS * sizeof( Uint32 ), (Ptr*)&m_pFourierDescrMeanAcc, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, NUM_COEFFS * sizeof( float ), (Ptr*)&m_pFourierDescrVarAcc, CVisBufferManager::BUF_FAST );

	// Acquire memory for the coefficient variance and mean value tables
	CVisBufferManager::Instance()->RequestBuffer( this, NUM_COEFFS * sizeof( Uint16 ), (Ptr*)&m_pFourierDescrMean, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, NUM_COEFFS * sizeof( Uint32 ), (Ptr*)&m_pFourierDescrVar, CVisBufferManager::BUF_FAST );
	
	ClearCalibrationData();

	m_unNumRelevantPairs = 15;
}
	
// *************************************************************************

void CVisFourierClassifier::DoProcessing()
{
	Uint16 *					pFourierCoeff = (Uint16*)m_iportFourierData.GetBuffer();
	PotatoList *				pList = (PotatoList *) m_iportObjectsList.GetBuffer();

	switch( m_eOperationMode )
	{
	case OP_CALIBRATION:
		// In calibration mode, we just have to add the coefficients to the accumulators
		AddToAccumulators( pFourierCoeff, m_pFourierDescrMeanAcc, m_pFourierDescrVarAcc );		
		break;

	case OP_CLASSIFICATION:
		// First determine the difference of this potato's form from the learned ones.
		Uint32 dist;
		Uint32 index;
		Uint32 curframe;

		index = pList->unCurrentPotatoId;
		curframe = pList->pObjects[index].unCurrentImageNum;
		dist = CalculateDifference( pFourierCoeff, m_pFourierDescrMean, m_pFourierDescrVar );

		// convert fixed point format
		ASSERT( CVisFourierDescriptor::COEFF_FRACTIONAL_BITS < PotatoObject::FP_FRACTIONAL_BITS );
		dist = dist << ( PotatoObject::FP_FRACTIONAL_BITS - CVisFourierDescriptor::COEFF_FRACTIONAL_BITS );
		pList->pObjects[index].unpClassificationForm[curframe] = dist;
		
		// DEBUG:
		//LogMsg("Entered for potato %d @ %d: %d -> %f", index, curframe, dist, fp2float(dist, 16 ) );
		break;

	default:
		break;
	}
}
	
// *************************************************************************

void CVisFourierClassifier::ClearCalibrationData()
{
	for ( Int32 i=0; i<NUM_COEFFS; i++ )
	{
		m_pFourierDescrMeanAcc[i] = 0;
		m_pFourierDescrVarAcc[i] = 0;
		m_pFourierDescrMean[i] = 0;
		m_pFourierDescrVar[i] = 0;
	}

	m_unNumSamples = 0;
}

// *************************************************************************	

Uint32 CVisFourierClassifier::GetCalibrationData( Ptr pBuffer )
{
	Uint32 * p = (Uint32 *)pBuffer;

	
	// Copy the calibration data. Note: use the startcopy command even for single values, since
	// that may otherwise cause cache troubles on the DSP.
	WaitCopy( StartCopy( (Ptr) p,					&m_unNumSamples, sizeof(Uint32) ) );
	WaitCopy( StartCopy( (Ptr)(p + 1),				m_pFourierDescrMeanAcc, NUM_COEFFS * sizeof( Uint32 ) ));
	WaitCopy( StartCopy( (Ptr)(p + 1 + NUM_COEFFS), m_pFourierDescrVarAcc, NUM_COEFFS * sizeof( float ) ));

	return GetCalibrationDataSize();
}

// *************************************************************************

Uint32 CVisFourierClassifier::SetCalibrationData( const Ptr pBuffer )
{
	Uint32 * p = (Uint32 *)pBuffer;
	
	// Copy the calibration data. Note: use the startcopy command even for single values, since
	// that may otherwise cause cache troubles on the DSP.
	WaitCopy( StartCopy( &m_unNumSamples,			(Ptr) p,					sizeof( Uint32 ) ));
	WaitCopy( StartCopy( m_pFourierDescrMeanAcc,	(Ptr)(p + 1),				NUM_COEFFS * sizeof( Uint32 ) ));
	WaitCopy( StartCopy( m_pFourierDescrVarAcc,		(Ptr)(p + 1 + NUM_COEFFS),	NUM_COEFFS * sizeof( float ) ));

	return GetCalibrationDataSize();
}

// *************************************************************************

Uint32 CVisFourierClassifier::GetCalibrationDataSize()
{
	return ( 1 + 2*NUM_COEFFS) * sizeof( Uint32 );
}

// *************************************************************************

void CVisFourierClassifier::CreateClassificationData()
{
	// Just calculate the mean and variance values.
	CalculateMeanAndVar(	m_unNumSamples, 
							m_pFourierDescrMeanAcc, m_pFourierDescrVarAcc, 
							m_pFourierDescrMean, m_pFourierDescrVar );
}

// *************************************************************************

Int32 CVisFourierClassifier::CalculateDifference( const Uint16 * restrict pCoefficients, const Uint16 * restrict pMean, const Uint32 * restrict pVar )
{
	Uint32 i, j;
	
	_nassert((int)(pCoefficients)%8  == 0);
	_nassert((int)(pMean)%8  == 0);
	_nassert((int)(pVar)%8  == 0);

#ifdef _DEBUG
	Uint32 * pDiff = (Uint32*)m_oportDifference.GetBuffer();
	for ( i=0; i<NUM_COEFFS; pDiff[i] = 0, i++);
#endif
		
	// Check the relevant pairs number. Can't be bigger than the number of pairs
	// available.
	if (m_unNumRelevantPairs > (NUM_COEFFS - 1) / 2)
	{
		m_unNumRelevantPairs = (NUM_COEFFS - 1) / 2;
	}
	
	// Calculate Difference to Calibration	
	Int32 unDiff = 0;
	Int32 unTmp;
	
	// Take m_unNumRelevantPairs coefficients from either side of the spectrum; the upper side
	// of the spectrum being the same as the negative side, actually.
	// Don't take c0, since it's the DC value and 0 anyway.
	// c(1) corresponds to c(NUM_COEFFS-1), c(2) corresponds to c(NUM_COEFFS-2) etc...
	// Remember, these are fixed point variables and thus must be shifted after multiplication.
	for (i = 1, j=NUM_COEFFS-1; i< 1 + m_unNumRelevantPairs; i++, j--)
	{
		// Lower side of spectrum
		// -----------------------------
		// Calculate deviation to mean
		unTmp = (Int32)pCoefficients[i] - (Int32)pMean[i];

		// square and divide by variance (the variance is already inverted, so we
		// can apply a normal multiplication).
		unTmp = unTmp * unTmp * pVar[i] >> CVisFourierDescriptor::COEFF_FRACTIONAL_BITS;
		unDiff += unTmp;

#ifdef _DEBUG
		pDiff[i] = unTmp << ( PotatoObject::FP_FRACTIONAL_BITS - CVisFourierDescriptor::COEFF_FRACTIONAL_BITS );		
#endif

		// Upper side of spectrum
		// -----------------------------
		// Calculate deviation to mean
		unTmp = (Int32)pCoefficients[j] - (Int32)pMean[j];

		// square and divide by variance (the variance is already inverted, so we
		// can apply a normal multiplication).
		unTmp = unTmp * unTmp * pVar[j] >> CVisFourierDescriptor::COEFF_FRACTIONAL_BITS;
		unDiff += unTmp;

#ifdef _DEBUG
		pDiff[j] = unTmp << ( PotatoObject::FP_FRACTIONAL_BITS - CVisFourierDescriptor::COEFF_FRACTIONAL_BITS );		
#endif
	}

	return unDiff ;
}
	
// *************************************************************************
	
void CVisFourierClassifier::AddToAccumulators( const Uint16 * restrict pCoefficients, Uint32 * restrict pMeanAcc, float * restrict pVarAcc )
{
	Uint32 m;
	float q;
	
	_nassert((int)(pCoefficients)%8  == 0);
	_nassert((int)(pMeanAcc)%8  == 0);
	_nassert((int)(pVarAcc)%8  == 0);
	
	for ( Uint32 i=0; i<NUM_COEFFS; i++)
	{
		// Read the value 
		m = (Uint32)pCoefficients[i];

		// Add to the mean accumulator
		pMeanAcc[i] += m;

		// Add to the variance accumulator. Note: these numbers are fixed point and
		// thus must be shifted when being multiplied!
		//q = ( m * m + ) >> ( CVisFourierDescriptor::COEFF_FRACTIONAL_BITS );
		q = (float)m / (float)( 1 << CVisFourierDescriptor::COEFF_FRACTIONAL_BITS );
		q = q*q;
		pVarAcc[i]  += q;
	}

	m_unNumSamples++;
}

// *************************************************************************

void CVisFourierClassifier::CalculateMeanAndVar( const Uint32 unNumSamples, const Uint32 * restrict pMeanAcc, const float * restrict pVarAcc, Uint16 * restrict pMean, Uint32 * restrict pVar )
{
	Uint32 m;
	Uint32 i;

	float fm, fv;
	Uint32 f_inv;

	// bail out if no data available
	if ( unNumSamples == 0 )
		return;

	// Calculate the variance and mean values.
	for ( i=1; i<NUM_COEFFS; i++)
	{
		// calculate mean
		m = (pMeanAcc[i]) / unNumSamples;
		pMean[i] = m;

		fm = (float)m / (float)(1<<CVisFourierDescriptor::COEFF_FRACTIONAL_BITS);		
		//fv = (float)pVarAcc[i] / (float)(1<<CVisFourierDescriptor::COEFF_FRACTIONAL_BITS);
		fv = pVarAcc[i];
		fv = fv / (float)unNumSamples - (fm*fm);
		if (fv == 0)
			fv = (float)1e-8;

		// Now invert that value and convert to integer
		f_inv = (Uint32)( 1 / fv );

		// Limit the inverted value somewhere.
		if ( f_inv > 10000000 )
			f_inv = 10000000;

		// And store.
		pVar[i] = f_inv;

	}
#ifdef _WRITETOFILE
	WriteIntValuesToFile( (short*)pMean, NUM_COEFFS, false, "_m.m", "m", true );
	WriteLongValuesToFile( (Int32*)pVar, NUM_COEFFS, false, "_v.m", "v", true );
#endif

}

// *************************************************************************

