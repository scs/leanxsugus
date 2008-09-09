
#include "classVisRGBToGray.h"

// *************************************************************************

CVisRGBToGray::CVisRGBToGray( const Char * strName )
	:	CVisComponent( "RGB2Gray", strName ),
		m_iportRGBInput("input", CVisPort::PDT_32BPP_RGB ),
		m_oportGrayOutput("output", CVisPort::PDT_8BPP_GRAY, CVisOutputPort::OUTPORT_LARGE )		
{
	m_iportRGBInput.Init( this );
	
	m_oportGrayOutput.Init( this );
}

// *************************************************************************

void CVisRGBToGray::Prepare()
{
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * 4, (Ptr*)&m_pRGBLine_0, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * 4, (Ptr*)&m_pRGBLine_1, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * 4, (Ptr*)&m_pRGBLine_2, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * 3, (Ptr*)&m_pGrayLine_0, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * 3, (Ptr*)&m_pGrayLine_1, CVisBufferManager::BUF_FAST );
	CVisBufferManager::Instance()->RequestBuffer( this, m_unResultWidth * 3, (Ptr*)&m_pGrayLine_2, CVisBufferManager::BUF_FAST );
}

// *************************************************************************

void CVisRGBToGray::DoProcessing()
{
	Int lines;
	
	// Write back the cache, so everything is read anew.
	m_iportRGBInput.CacheInvalidate();
	m_oportGrayOutput.CacheInvalidate();
	
	// Acquire the image buffers
	const Uint32 * pRGBImage	= (const Uint32*)m_iportRGBInput.GetBuffer();
	Uint8 * pGrayImage			= (Uint8*)m_oportGrayOutput.GetBuffer();	
	
	int c1,c2;
	
	// ***************************************************************
	//  Process the image line by line
	// ***************************************************************
	Uint32 offs = 0;

	// Prolog I: Get the first line
	offs = 0;
	c1 = StartCopy( m_pRGBLine_2, (Ptr)(pRGBImage + offs), m_unResultWidth*4 );
	WaitCopy(c1);
	
	// Prolog II: Process the first line (line=0) and get the next.
	offs = 0;
	c1 = StartCopy( m_pRGBLine_0, (Ptr)(pRGBImage + offs + m_unResultWidth), m_unResultWidth * 4 );
	RGBtoGray( m_pRGBLine_2, (Uint32*)(m_pGrayLine_2), m_unResultWidth );
	WaitCopy(c1);
	
	// Kernel.
	for ( lines = 1; lines < (m_unResultHeight-3); lines+=3)
	{		
		// I
		offs = lines * m_unResultWidth;
		c1 = StartCopy( m_pRGBLine_1, (Ptr)(pRGBImage + offs + m_unResultWidth ), m_unResultWidth * 4 );
		c2 = StartCopy( (Ptr)(pGrayImage + offs - m_unResultWidth), m_pGrayLine_2, m_unResultWidth );
		RGBtoGray( m_pRGBLine_0, (Uint32*)(m_pGrayLine_0), m_unResultWidth );
		WaitCopy( c1 );
		WaitCopy( c2 );
		
		// II
		offs += m_unResultWidth;
		c1 = StartCopy( m_pRGBLine_2, (Ptr)(pRGBImage + offs + m_unResultWidth ), m_unResultWidth * 4 );
		c2 = StartCopy( (Ptr)(pGrayImage + offs - m_unResultWidth ), m_pGrayLine_0, m_unResultWidth );
		RGBtoGray( m_pRGBLine_1, (Uint32*)(m_pGrayLine_1), m_unResultWidth );
		WaitCopy( c1 );
		WaitCopy( c2 );			
		
		// III
		offs += m_unResultWidth;
		c1 = StartCopy( m_pRGBLine_0, (Ptr)(pRGBImage + offs + m_unResultWidth ), m_unResultWidth * 4 );
		c2 = StartCopy( (Ptr)(pGrayImage + offs - m_unResultWidth ), m_pGrayLine_1, m_unResultWidth );
		RGBtoGray( m_pRGBLine_2, (Uint32*)(m_pGrayLine_2), m_unResultWidth );
		WaitCopy( c1 );
		WaitCopy( c2 );		
	}

	// Epilogue: Copy the last processed line out
	offs += m_unResultWidth;
	c2 = StartCopy( (Ptr)(pGrayImage + offs - m_unResultWidth ), m_pGrayLine_2, m_unResultWidth );
	WaitCopy( c2 );			
	
	// Do the remaining lines.
	for ( ; lines<m_unResultHeight; lines++ )
	{
		Uint32 offs = lines * m_unResultWidth;
		QuickCopy( m_pRGBLine_0, (Ptr)(pRGBImage + offs), m_unResultWidth * 4 );
		RGBtoGray( m_pRGBLine_0, (Uint32*)m_pGrayLine_0, m_unResultWidth );
		QuickCopy( (Ptr)(pGrayImage + offs), m_pGrayLine_0, m_unResultWidth );
	}

	// Write back the cache, so everything is stored.
	m_oportGrayOutput.CacheInvalidate();
}

// *************************************************************************

void CVisRGBToGray::RGBtoGray( const Uint32 * restrict pRGBImage, Uint32 * restrict pGrayImage, const Uint32 numPixels )
{
	// Define a multiplication mask for use with the DOTP intrinsics. Multiply R, G
	// and B by 85 and A by 0. Then, divide it by 256, so that we get a factor of
	// 1/3 ( = 85/256).
	//#define MULTIPLYMASK 0x00555555		

	// Out = R*0.3 + G*0.59 + B*0.11
	#define MULTIPLYMASK 0x001C964C


	Uint32 n1, n2, n3, n4;
	Uint32 m1, m2;

	for( unsigned int i=0; i<numPixels ; i+=4)
	{
		
		n1 = _dotpu4 ( pRGBImage[i], MULTIPLYMASK ) >> 8;
		n2 = _dotpu4 ( pRGBImage[i+1], MULTIPLYMASK ) >> 8;		
		n3 = _dotpu4 ( pRGBImage[i+2], MULTIPLYMASK ) >> 8;
		n4 = _dotpu4 ( pRGBImage[i+3], MULTIPLYMASK ) >> 8;
		
		m1 = _pack2(n2,n1);
		m2 = _pack2(n4,n3);

		pGrayImage[i>>2] = _packl4(m2,m1);
	}
}

// *************************************************************************
