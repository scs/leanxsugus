
#include "classVisObject.h"
#include "classVisObjectManager.h"

#include <stdio.h>
#include <string.h>

// Include the MFC stuff here, if we're on windows.
#ifdef _WINDOWS

#include "../VisionTest/stdafx.h"

#else

#include <csl_cache.h>
#include "../libDebug.h"
#include "../classProfiler.h"

#endif

const char * CVisObject::csClassTypesString[CT_NUM_TYPES]=
	{	"Vision",
		"BufferManager",
		"ObjectManager",
		"Component",
		"Port",
		"Property",
		"Math" };


// *************************************************************************


CVisObject::CVisObject( const char * strName, const ClassType type)
{
	m_ctClassType = type;

	if ( type != CT_MATH )
	{
		SetName( strName );
			
		// Register this object, but only if it isn't the manager itself.
		// Also, math objects aren't registered.
		if ( type != CT_OBJECT_MANAGER )
			CVisObjectManager::Instance()->AddObject( this );
	}
	else
	{
		// math objects are not registered and don't have a name.
		SetName("");
	}
}


// *************************************************************************
/*
CVisObject::~CVisObject()
{
}
*/
// *************************************************************************

void CVisObject::SetName( const Char * strName )
{
	//StringCopy( strName, m_strName, OBJ_NAME_MAX_CHARS );
	m_strName = strName;
}
 
// *************************************************************************

const char * CVisObject::GetName() const
{
	return m_strName;
}

// *************************************************************************

bool CVisObject::HasName( const Char * strName ) const
{
	return StringCompare( m_strName, strName, OBJ_NAME_MAX_CHARS );
}

// *************************************************************************

CVisObject::ClassType CVisObject::GetClassType() const
{
	return m_ctClassType;
}

// *************************************************************************

const Char * CVisObject::GetStrClassType() const
{
	return csClassTypesString[ GetClassType() ];
}

// *************************************************************************

void CVisObject::StringCopy( const Char * src, Char * dest, Uint32 maxChars) const
{
	Uint32 i;
	
	i = 0;
	
	do
	{
		dest[i] = src[i];
		i++;
	} while ( ( src[i] != 0 ) && ( i < (maxChars-1) ));
	
	// terminate string
	dest[i] = 0;
}

// *************************************************************************

bool CVisObject::StringCompare( const Char * src, const Char * dest, Uint32 maxChars) const
{
	Uint32 i;
	
	i = 0;
	
	do
	{
		if ( src[i] != dest[i])
			return false;		
		
		// If string ends, return true (don't have to check dest, since src[i] == dest[i] here	
		if (src[i] == 0)
			return true;
						
		i++;		
	} while (i < maxChars);
	
	return true;	
}

// *************************************************************************

void CVisObject::ClearList( CVisObject ** ppList, Uint32 unNumEntries ) const
{
	Uint32 i;
	
	for (i=0; i<unNumEntries; i++)
		ppList[i] = NULL;
}

// *************************************************************************

bool CVisObject::AddObjectToList( CVisObject ** ppList, Uint32 unNumEntries, CVisObject * pObject ) const
{
	Uint32 i = 0;
	
	while ( ppList[i] != NULL )
	{
		i++;
		if ( i == unNumEntries )
			return false;
	}
	
	ppList[i] = pObject;
	return true;
}
	
// *************************************************************************

bool CVisObject::GetObjectFromList( CVisObject ** ppList, Uint32 unNumEntries, CVisObject ** pObject, const char * strName ) const
{
	Uint32 i=0;
	
	while ( i<unNumEntries )
	{
		if (ppList[i] != NULL)
		{
			if ( StringCompare(ppList[i]->GetName(), strName, OBJ_NAME_MAX_CHARS ) )
			{
				* pObject = ppList[i];
				return true;
			}
		}
		
		i++;
	}
	
	return false;
}

// *************************************************************************

bool CVisObject::GetNextObjectFromList( CVisObject ** ppList, Uint32 unNumEntries, CVisObject ** pObject, Int & nID ) const
{
	// Abort if we're at the end of the list
	if ((unsigned)(nID + 1) >= unNumEntries)
		return false;
		
	// Abort if the next entry is empty
	if (ppList[nID+1] == NULL)
		return false;
		
	// increment the id. If it was -1, it will now point to the first object.
	nID++;		
		
	// Return the object
	*pObject = ppList[nID];
	return true;
}

// *************************************************************************

void CVisObject::MemSet( Ptr pDest, Uint8 unValue, Uint32 unSize ) const
{
	Uint32 n32;
	Uint8 * p = (Uint8*)pDest;
	Uint32 size = unSize;
	
	n32 = ( unValue | (unValue << 8 ) | (unValue << 16 ) | (unValue << 24 ) );
	
	while( size > 3 )
	{
		*((Uint32*)p) = n32;
		
		size -= 4;
		p += 4;
	}
	
	while( size > 0 )
	{
		*((Uint8*)p) = unValue;
		
		size--;
		p += 1;
	}

}

// *************************************************************************

void CVisObject::QuickCopy(  Ptr dest, Ptr source, Uint32 size ) 
{
	if ( size < 0xFFFF )
		memcpy( dest, source, size );
	else
		WaitCopy( StartCopy( dest, source, size ) );
}

// *************************************************************************

Uint32 CVisObject::StartCopy( Ptr dest, Ptr source, Uint32 size )
{
#ifdef _WINDOWS
	CopyRequest * req = new CopyRequest;
	req->unType = COPY_1D1D;
	req->pSource = source;
	req->pDest = dest;
	req->unSize = size;
	
	// Do some of the copies right now, postpone the rest
	if ( rand() & 0x1 )
	{
		DoCopy( req );
		delete req;
		return COPY_DONE;
	}
	else
	{
		m_lCopyRequest.AddTail( req );
		return (Uint32)req;
	}

#else

	return edmaStartCopy( dest, source, size, FALSE );
	
#endif
}

// *************************************************************************

Uint32 CVisObject::StartCopy2D( Uint32 type, Ptr dest, Ptr source, Uint16 lineLen, Uint16 lineCnt, Uint16 linePitch ) 
{
#ifdef _WINDOWS
	CopyRequest * req = new CopyRequest;
	req->unType = type;
	req->pSource = source;
	req->pDest = dest;
	req->unLineLen = lineLen;
	req->unLineCount = lineCnt;
	req->unLinePitch = linePitch;
	
	// Do some of the copies right now, postpone the rest
	if ( rand() & 0x01 )
	{
		DoCopy( req );
		delete req;
		return COPY_DONE;
	}
	else
	{
		m_lCopyRequest.AddTail( req );
		return (Uint32)req;
	}

#else

	return edmaStartCopy2D( type, dest, source, lineLen, lineCnt, linePitch, FALSE );

#endif
}

// *************************************************************************

void CVisObject::WaitCopy( Uint32 unCopyId ) 
{
	if ( unCopyId == COPY_DONE )
		return;

#ifdef _WINDOWS
	POSITION pos;
	CopyRequest * req;

	req = (CopyRequest*)unCopyId;

	// Find the copy request in the list and remove it.
	pos = m_lCopyRequest.Find( req );
	if ( pos == NULL )
	{
		LogMsg("WaitCopy: Illegal copyId!");
		ASSERT(0);
	}
	m_lCopyRequest.RemoveAt( pos );

	// Do the actual copy.
	DoCopy( req );

	// delete the request
	delete req;

#else

	edmaWaitCopy( unCopyId );

#endif
}

// *************************************************************************
#ifdef _WINDOWS

void CVisObject::DoCopy( CopyRequest * req )
{
	Uint32 i;
	Uint8 * src = (Uint8*)req->pSource;
	Uint8 * dst = (Uint8*)req->pDest;

	switch ( req->unType )
	{
	case COPY_1D1D:
		memcpy( req->pDest, req->pSource, req->unSize );
		break;

	case COPY_1D2D:
		for ( i=0; i<req->unLineCount; i++ )
		{
			memcpy( dst, src, req->unLineLen );
			src += req->unLineLen;
			dst += req->unLinePitch;
		}
		break;
		
	case COPY_2D1D:
		for ( i=0; i<req->unLineCount; i++ )
		{
			memcpy( dst, src, req->unLineLen );
			src += req->unLinePitch;
			dst += req->unLineLen;
		}
		break;
		
	case COPY_2D2D:
		for ( i=0; i<req->unLineCount; i++ )
		{
			memcpy( dst, src, req->unLineLen );
			dst += req->unLinePitch;
			src += req->unLinePitch;
		}
		break;
	}		
}
#endif

// *************************************************************************

void  CVisObject::WritebackCache( const Ptr p, const Uint32 unSize ) const
{
#ifndef _WINDOWS

	#define MAXBLOCKSIZE 0x8000
	
	Uint8 * pNext = (Uint8*)p;
	Uint32 	unRemaining = unSize;
	
	
	while (unRemaining > MAXBLOCKSIZE )
	{
		CACHE_wbL2( pNext, MAXBLOCKSIZE, CACHE_WAIT );
		pNext += MAXBLOCKSIZE;
		unRemaining -= MAXBLOCKSIZE;
	}
	
	CACHE_wbL2( pNext, unRemaining, CACHE_WAIT );
	
#endif	
}

// *************************************************************************

void  CVisObject::InvalidateCache( const Ptr p, const Uint32 unSize ) const
{
#ifndef _WINDOWS

	#define MAXBLOCKSIZE 0x8000
	
	Uint8 * pNext = (Uint8*)p;
	Uint32 	unRemaining = unSize;
	
	
	while (unRemaining > MAXBLOCKSIZE )
	{
		CACHE_invL2( pNext, MAXBLOCKSIZE, CACHE_WAIT );
		pNext += MAXBLOCKSIZE;
		unRemaining -= MAXBLOCKSIZE;
	}
	
	CACHE_invL2( pNext, unRemaining, CACHE_WAIT );
	
#endif
}	

// *************************************************************************

void CVisObject::LogMsg(const char* strFormat, ...) const
{	
	char str[256];

	// Print message to the string
	va_list ap;
	va_start(ap, strFormat);
	vsprintf(str, strFormat, ap);	
  	va_end(ap);
  	
#ifdef _WINDOWS
	// Log whole message
	TRACE("%s:%s > %s\n", GetStrClassType(), GetName(), str );
#else
	// Log whole message
	dbgLog("%s:%s > %s", GetStrClassType(), GetName(), str );
#endif
}

// *************************************************************************

Int CVisObject::NewProfileTask( const Char * strTaskName ) const
{
#ifdef _WINDOWS
	return 0;
#else
	return CProfiler::Instance()->NewProfileTask( strTaskName );
#endif	
}

// *************************************************************************

void CVisObject::StartProfileTask( Int tasknum ) const
{
#ifndef _WINDOWS
	CProfiler::Instance()->StartProfileTask( tasknum );
#endif
}

// *************************************************************************

void CVisObject::StopProfileTask( Int tasknum ) const
{
#ifndef _WINDOWS
	CProfiler::Instance()->StopProfileTask( tasknum );
#endif
}

// *************************************************************************
	
#ifdef _WRITETOFILE

void CVisObject::WriteIntValuesToFile(	Int16 * mValues, 
										Uint32 numValues, 
										bool bComplex, 
										CString strFile, 
										CString strVar, 
										bool bFirst )
{

	CFile file;
	CString str;

	CString strPath;

	strPath.Format("P:\\Samro\\200306_Potato_Quality_RT_Prototype\\Implementation\\Software\\DSP\\FlexiVision\\VisionTest\\matlab\\%s", strFile );

	if (bFirst)
	{
		if ( ! file.Open( strPath, CFile::modeCreate | CFile::modeWrite) )
			return;
		str.Format("clear %s;\n%s=[];\n", strVar, strVar );
		file.Write( str, str.GetLength() );
	}
	else
	{
		if ( ! file.Open( strPath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate ) )
			return;
		file.SeekToEnd();
	}	

	str.Format( "%s = [%s ; ", strVar, strVar );
	file.Write( str, str.GetLength() );

	for ( Uint32 i=0; i<numValues; i++ )
	{
		if (bComplex)
		{
			str.Format( " (%d+%di)", mValues[ i*2 ], mValues[ i*2 + 1 ]);
			file.Write( str, str.GetLength() );
		}
		else
		{
			str.Format( " %d", mValues[ i ] );
			file.Write( str, str.GetLength() );
		}
	}

	str = "];\n";
	file.Write( str, str.GetLength() );

	file.Close();
}

// *************************************************************************

void CVisObject::WriteLongValuesToFile(	Int32 * mValues, 
										Uint32 numValues, 
										bool bComplex, 
										CString strFile, 
										CString strVar, 
										bool bFirst )
{

	CFile file;
	CString str;

	CString strPath;

	strPath.Format("P:\\Samro\\200306_Potato_Quality_RT_Prototype\\Implementation\\Software\\DSP\\FlexiVision\\VisionTest\\matlab\\%s", strFile );

	if (bFirst)
	{
		if ( ! file.Open( strPath, CFile::modeCreate | CFile::modeWrite) )
			return;
		str.Format("clear %s;\n%s=[];\n", strVar, strVar );
		file.Write( str, str.GetLength() );
	}
	else
	{
		if ( ! file.Open( strPath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate ) )
			return;
		file.SeekToEnd();
	}	

	str.Format( "%s = [%s ; ", strVar, strVar );
	file.Write( str, str.GetLength() );

	for ( Uint32 i=0; i<numValues; i++ )
	{
		if (bComplex)
		{
			str.Format( " (%d+%di)", mValues[ i*2 ], mValues[ i*2 + 1 ]);
			file.Write( str, str.GetLength() );
		}
		else
		{
			str.Format( " %d", mValues[ i ] );
			file.Write( str, str.GetLength() );
		}
	}

	str = "];\n";
	file.Write( str, str.GetLength() );

	file.Close();
}

// *************************************************************************


void CVisObject::WriteFloatValuesToFile(	float * mValues, 
											Uint32 numValues, 
											bool bComplex, 
											CString strFile, 
											CString strVar, 
											bool bFirst )
{

	CFile file;
	CString str;

	CString strPath;

	strPath.Format("P:\\Samro\\200306_Potato_Quality_RT_Prototype\\Implementation\\Software\\DSP\\FlexiVision\\VisionTest\\matlab\\%s", strFile );

	if (bFirst)
	{
		if ( ! file.Open( strPath, CFile::modeCreate | CFile::modeWrite) )
			return;
		str.Format("clear %s;\n%s=[];\n", strVar, strVar );
		file.Write( str, str.GetLength() );
	}
	else
	{
		if ( ! file.Open( strPath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate ) )
			return;
		file.SeekToEnd();
	}	

	str.Format( "%s = [%s ; ", strVar, strVar );
	file.Write( str, str.GetLength() );

	for ( Uint32 i=0; i<numValues; i++ )
	{
		if (bComplex)
		{
			str.Format( " (%f+%fi)", mValues[ i*2 ], mValues[ i*2 + 1 ]);
			file.Write( str, str.GetLength() );
		}
		else
		{
			str.Format( " %f", mValues[ i ] );
			file.Write( str, str.GetLength() );
		}
	}

	str = "];\n";
	file.Write( str, str.GetLength() );

	file.Close();
}
#endif

// *************************************************************************
