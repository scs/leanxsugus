

#include "tstRingBuffer.h"


char		c;

const char 	* cstr = "This is a test string";
char		str[64];

Bool 	RingBufferTestChar()
{
	#define BUFSIZE 	32
	#define TESTSIZE 	7
	#define NUMTESTS	100

	char 			buf[BUFSIZE];
	RingBuffer 		ringbuffer;
	Int 			i,j;
	char 			c;
	const char *	teststr = "012345678901234567890";
	
	rbfCreate( &ringbuffer, buf, BUFSIZE );
	
	for ( i=0; i<NUMTESTS; i++)
	{
		if ( i==31 )
			c = 10;
			
		if ( rbfWrite( &ringbuffer, teststr, TESTSIZE ) != TESTSIZE)
			return FALSE;
			
		for	(j=0; j<TESTSIZE; j++)
		{
			if ( ! rbfPeekChar( &ringbuffer, &c ) ) 
				return FALSE;
				
			if ( c != teststr[j] )
				return FALSE;
				
			if ( rbfNumFull( &ringbuffer ) != (TESTSIZE - j) )
				return FALSE;
				
			if ( rbfNumEmpty( &ringbuffer ) != (BUFSIZE - TESTSIZE + j - 1 ) )
				return FALSE;
				
			rbfReadChar( &ringbuffer, &c );			
		}
		
		// Check that the buffer is empty now.
		if ( rbfPeekChar( &ringbuffer, &c ) )
			return FALSE;
		if ( rbfNumFull( &ringbuffer ) != 0 )
			return FALSE;				
		if ( rbfNumEmpty( &ringbuffer ) != (BUFSIZE-1) )
			return FALSE;
	}
	
	return TRUE;
}

Bool	RingBufferTest()
{
	RingBuffer 	ringbuffer;
	char 		buf[6];
	Int i,j;
	Int len = strlen(cstr);
	
	
	rbfCreate( &ringbuffer, buf, 6 );

	// *********************************************************
	//  Test 1
	// *********************************************************	
	// Single chars
	assert( rbfNumEmpty( &ringbuffer ) == 5 );
	assert( rbfNumFull( &ringbuffer ) == 0 );
	
	rbfWriteChar( &ringbuffer, 'a' );
	assert( rbfNumEmpty( &ringbuffer ) == 4 );
	assert( rbfNumFull( &ringbuffer ) == 1 );
	assert( rbfReadChar( &ringbuffer, &c ));
	assert( c == 'a' );
	
	for (i=0; i<10; i++)
	{
		rbfWriteChar( &ringbuffer, 'a'+i );
		assert( rbfNumEmpty( &ringbuffer ) == 4 );
		assert( rbfNumFull( &ringbuffer ) == 1 );
		assert( rbfReadChar( &ringbuffer, &c ));
		assert( c == 'a'+i );
	}
	
	for (i=0; i<5; i++)
	{
		rbfWriteChar( &ringbuffer, 'j'+i );
		assert( rbfNumFull( &ringbuffer ) == i+1 );
	}
	assert( ! rbfWriteChar( &ringbuffer, 'z' ));
	for (i=0; i<5; i++)
	{
		rbfReadChar( &ringbuffer, &c );
		assert( c == 'j' + i );
	}
	assert( rbfNumFull( &ringbuffer ) == 0 );
	
	// *********************************************************
	//  Test 2
	// *********************************************************	
	// Strings
	
	// reset rbf
	rbfDelete(&ringbuffer);
	rbfCreate( &ringbuffer, buf, 6 );
	
	// no wrap-around
	assert( rbfWrite( &ringbuffer, cstr, len) == 5);
	assert( rbfNumFull( &ringbuffer ) == 5 );
	assert( rbfRead( &ringbuffer, str, 10 ) == 5 );
	for (i=0; i<5; i++)	assert( str[i] == cstr[i] );
	for (i=0; i<5; i++)	assert( ringbuffer.Buffer[i] == cstr[i] );
		
	// Do the same test again to force a wrap-around
	assert( rbfWrite( &ringbuffer, cstr, len) == 5);
	assert( rbfNumFull( &ringbuffer ) == 5 );
	assert( rbfRead( &ringbuffer, str, 10 ) == 5 );
	for (i=0; i<5; i++)	assert( str[i] == cstr[i] );
	
	// and repeat some times to ensure long-term stability
	for (j=0; j<15; j++)
	{
		assert( rbfWrite( &ringbuffer, cstr+j, len) == 5);
		assert( rbfNumFull( &ringbuffer ) == 5 );
		assert( rbfRead( &ringbuffer, str, 10 ) == 5 );
		for (i=0; i<5; i++)	assert( str[i] == cstr[i+j] );
	}	
	
	rbfDelete( &ringbuffer );
	
	LOG_printf( &trace, "RingBuffer tested succesfully. ");	
	
	return TRUE;
}
