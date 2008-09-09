
#include "classVisTracker.h"

CVisTracker::CVisTracker( const Char * strName )
		:	CVisComponent( "Tracker", strName ),
			m_iportLabelObjects( "objLabels", CVisPort::PDT_DATA ),
			m_oportPotatoObjects( "objPotatoes", CVisPort::PDT_DATA ),
			m_propMillimeterPerPixel( "mmPerPixel" ),
			m_propSearchRadius2( "SearchRadius2" ),
			m_propInsertionZone( "InsertionZone" ),
			m_propDropZone( "DropZone" ),
			m_propValidEjectionZone( "EjectionZone" ),
			m_propDropTime( "DropTime" )
	
{
	m_iportLabelObjects.Init( this );
	m_oportPotatoObjects.Init( this );
	m_oportPotatoObjects.SetBufferSize( sizeof(PotatoList) );
	
	m_propMillimeterPerPixel.Init( this, CVisProperty::PT_FIXEDPOINT, (Ptr)&m_fpMillimeterPerPixel, 16 );
	m_propSearchRadius2.Init( this, CVisProperty::PT_INTEGER, (Ptr)&m_nSearchRadius2 );
	m_propInsertionZone.Init( this, CVisProperty::PT_INTEGER, (Ptr)&m_nInsertionZone );
	m_propDropZone.Init( this, CVisProperty::PT_INTEGER, (Ptr)&m_nDropZone );
	m_propValidEjectionZone.Init( this, CVisProperty::PT_INTEGER, (Ptr)&m_nValidEjectionZone );
	m_propDropTime.Init( this, CVisProperty::PT_INTEGER, (Ptr)&m_nDropTime );

	// Set the clear bit so that the list is erased before processing.
	m_bClearListNext = true;

	m_fpMillimeterPerPixel = F2FP( 0.9, 16 );
	m_fpConveyorSpeed = 0;//F2FP( 0.18, 16 );
	m_unCurrentFrameTime = 0;
	m_nSearchRadius2 = 6400;
	m_nDropTime = 1000;

	// The zone definitions
	m_nInsertionZone = 50;//200; 
	m_nValidEjectionZone = -110;//420;
	m_nDropZone = -145;//425;//450;

	// DEBUG:
#ifdef _WINDOWS
	m_nInsertionZone = 80;
#endif
}

// *************************************************************************

void CVisTracker::DoProcessing()
{
	PotatoList *					potList = (PotatoList *)m_oportPotatoObjects.GetBuffer();
	FastLabelObject *				labelObjects = (FastLabelObject*)m_iportLabelObjects.GetBuffer();

	// Clear the list if requested.
	if (m_bClearListNext)
	{
		m_bClearListNext = false;
		ClearList();
	}
	
	// Track all known objects.
	TrackObjects( labelObjects, potList->pObjects );
	
	// Add the new objects to the list of known objects.
	AddNewObjects( labelObjects, potList->pObjects );
}

// *************************************************************************

void CVisTracker::SetCurrentImageTime( Uint32 unMilliseconds )
{
	m_unCurrentFrameTime = unMilliseconds;
}

// *************************************************************************

void CVisTracker::SetConveyorSpeed( Uint32 fp16Speed )
{
	m_fpConveyorSpeed = fp16Speed;
}
					
// *************************************************************************

void CVisTracker::ClearList()
{
	PotatoList * pList = (PotatoList *)m_oportPotatoObjects.GetBuffer();

	for (int i=0; i<PotatoList::MAX_OBJECTS; i++)
	{
		pList->pObjects[i].bValid = false;
		pList->pObjects[i].unCurrentImageNum = 0;
	}
}
					
// *************************************************************************

void CVisTracker::TrackObjects( FastLabelObject * labelObjects, PotatoObject * potObjects )
{
	// Go through the list and track each object.
	for ( int i=0; i<PotatoList::MAX_OBJECTS; i++)
	{
		if ( potObjects[i].bValid )
		{
			Int32 predX;
			Int32 predY;
			Int	minDist = 0x0FFFFFFF;		// Set distance to far beyond possibility.
			Uint32 minDistIndex = 0;		// There is no label with index 0

			// Predict the potato's position
			potObjects[i].PredictPosition( m_unCurrentFrameTime, m_fpConveyorSpeed );
			predX = potObjects[i].nPredictedPos_mm_X;
			predY = potObjects[i].nPredictedPos_mm_Y;

			for ( Uint32 label=1; label<labelObjects[0].unNumObjects; label++)
			{
				Int		dist;
				Int		pos_mm_X;
				Int		pos_mm_Y;
				Int		deltaX;
				Int		deltaY;
				bool	tracked;
				bool	nearer;
				bool	found;
				
				// Read the label objects fixpoint coordinates, which are in meters,
				// and convert them to millimeters.
				pos_mm_X = (int)(labelObjects[label].pTransformedCoords->fpMx * 1000);
				pos_mm_Y = (int)(labelObjects[label].pTransformedCoords->fpMy * 1000);
				deltaX = pos_mm_X - predX;
				deltaY = pos_mm_Y - predY;
				
				dist = (deltaY*deltaY + deltaX*deltaX);
				
				// Pre-calculate some bools, so that we don't have to jump
				// (which prohibits a pipelined loop!)			
				tracked = labelObjects[label].bTracked;
				nearer = (dist<minDist);	
				found = (!tracked && nearer);

				// If this label has not been tracked before AND its distance is
				// smaller, temporarily store its index.
				if ( found )
				{
					minDist = dist;
					minDistIndex = label;
				}

			}

			// If the distance is smaller than the search radius, this is our object.
			if ( minDist < m_nSearchRadius2 )
			{
				// Store the index to the label
				potObjects[i].unCurrentLabel = minDistIndex;

				// Again, convert the label's fixedpoint meters to integer millimeters and store them as
				// last valid position.
				potObjects[i].nLastSeenPos_mm_X = (int)(labelObjects[minDistIndex].pTransformedCoords->fpMx * 1000);
				potObjects[i].nLastSeenPos_mm_Y = (int)(labelObjects[minDistIndex].pTransformedCoords->fpMy * 1000);
				
				// Store the time
				potObjects[i].unLastSeenTime_ms = m_unCurrentFrameTime;				

				// Mark the label as being tracked for this frame, so that it can't be
				// used for other objects.
				labelObjects[minDistIndex].bTracked = true;

				// Mark the potato object as tracked.
				potObjects[i].bTracked = true;
				
				// Calculate the currently seen size. The size given by the labeller is
				// in square pixels. We always calculate the mean of the last value and the
				// new one. This way, we'll get an average of all values at the end.
				Uint32 unCurLength;
				unCurLength = ( (labelObjects[minDistIndex].unBoundingRight - labelObjects[minDistIndex].unBoundingLeft) * m_fpMillimeterPerPixel  ) >> 16;
				potObjects[i].unPotatoLength = (potObjects[i].unPotatoLength + unCurLength) / 2;
				
				Uint32 unCurWidth;				
				unCurWidth = ( (labelObjects[minDistIndex].unBoundingBottom - labelObjects[minDistIndex].unBoundingTop) * m_fpMillimeterPerPixel  ) >> 16;
				potObjects[i].unPotatoWidth = ( potObjects[i].unPotatoWidth + unCurWidth ) / 2;
								
				// Now see if the object is in the valid ejection (aka valid drop) zone.
				if ( potObjects[i].nLastSeenPos_mm_Y < m_nValidEjectionZone )
					potObjects[i].bValidEjection = TRUE;		
					
				// Determine if the object's TRACKED position is in the drop zone and mark the object accordingly.
				if ( potObjects[i].nLastSeenPos_mm_Y < m_nDropZone )
					potObjects[i].bDropIt = TRUE;					
								
				// Done with this object.				
			}
			// If no label was found that matches the search radius, the object could not be tracked
			// for this frame.
			else	
			{
				Int nDeltaT;
				nDeltaT = (Int)(m_unCurrentFrameTime - potObjects[i].unLastSeenTime_ms); 

				// Mark the potato object as not tracked for this pass
				potObjects[i].bTracked = false;
				
				// Determine if the object's PREDICTED position is in the drop zone and thus should be dropped
				// Note: since the object could not be properly tracked within this frame, it actually may not
				// be ejected. This depends on whether the object has been seen in the validEjection zone.
				if ( predY < m_nDropZone )
					potObjects[i].bDropIt = TRUE;
					
				// See if the object could not be tracked for a too long time and DISCARD it if this
				// is the case.
				if ( nDeltaT > m_nDropTime )
					potObjects[i].bDiscard = TRUE;
			}			
				
		} // if it's a valid object
		
	} // for all list entries
}
					
// *************************************************************************
		
void CVisTracker::AddNewObjects( const FastLabelObject * labelObjects, PotatoObject *	potObjects )
{
	Int index = 0;
	
	// Go through the list of not yet tracked labels and see if there are any
	// new potato objects that we can add to the list.
	for ( Uint32 labelIndex=1; labelIndex < labelObjects[0].unNumObjects; labelIndex++)
	{
		Int pos_mm_Y = (int)(labelObjects[labelIndex].pTransformedCoords->fpMy * 1000);

		// If the label has not yet been tracked AND it is in the insertion zone, add it to the
		// objects list
		if (		( !labelObjects[labelIndex].bTracked ) 
				&&	( pos_mm_Y > m_nInsertionZone) )
		{		

			// Find the next free object entry
			while ( potObjects[index].bValid )
			{
				index++;
				
				// Abort if we've gone too far.
				if (index == PotatoList::MAX_OBJECTS)
				{
					LogMsg("No place for new objects!");
					return;
				}
			}
			
			// Transfer the information.
			
			// Clear the ejection related bools
			potObjects[index].bValidEjection = false;
			potObjects[index].bDropIt = false;
			potObjects[index].bDiscard = false;
		
			potObjects[index].unPotatoClass = 0;
			
			// Transfer the position from the label object
			potObjects[index].nLastSeenPos_mm_X = (int)(labelObjects[labelIndex].pTransformedCoords->fpMx * 1000);
			potObjects[index].nLastSeenPos_mm_Y = (int)(labelObjects[labelIndex].pTransformedCoords->fpMy * 1000);
			
			// Set the current time.
			potObjects[index].unLastSeenTime_ms = m_unCurrentFrameTime;
			
			potObjects[index].unLastSeenImage = 0;
			
			// We start with image 0.
			potObjects[index].unCurrentImageNum = 0;
			
			// The index of the current label object.
			potObjects[index].unCurrentLabel = labelIndex;

			// Reset the local bounding box
			potObjects[index].unLocalBoundingLeft = 0;
			potObjects[index].unLocalBoundingTop = 0;
			potObjects[index].unLocalBoundingRight = 0;
			potObjects[index].unLocalBoundingBottom = 0;
			
			// Store this object's size. The size must be converted from pixels to mm^2.
			potObjects[index].unPotatoLength = ( (labelObjects[labelIndex].unBoundingLeft - labelObjects[labelIndex].unBoundingRight) * m_fpMillimeterPerPixel  ) >> 16;
			potObjects[index].unPotatoWidth = ( (labelObjects[labelIndex].unBoundingBottom - labelObjects[labelIndex].unBoundingBottom) * m_fpMillimeterPerPixel  ) >> 16;
			
			// Enable the object at last
			potObjects[index].bValid = true;
			potObjects[index].bTracked = true;
			
		}
	}	
}

// *************************************************************************

void CVisTracker::DoneWithPotato( const Uint32 unPotatoId )
{
	PotatoList * pList = (PotatoList *)m_oportPotatoObjects.GetBuffer();
	PotatoObject * obj = &(pList->pObjects[unPotatoId]);
	
	// First check if the potato has really been tracked and classified for this frame.
	if ( ! obj->bTracked )
		return;

	// Move to the next seen image, so the classification results for the next frame 
	// go to the right place in the arrays.
	obj->unCurrentImageNum++;
	
	// Check that we haven't got too many frames
	if ( obj->unCurrentImageNum > PotatoObject::MAX_FRAMES_PER_POTATO-1 )
			obj->unCurrentImageNum = PotatoObject::MAX_FRAMES_PER_POTATO-1;	
}
					
// *************************************************************************

void PotatoObject::PredictPosition(	Uint32 unCurTime, Int fp16ConveyorSpeed)
{
	Uint32 deltaT;

	// Calculate the delta time in ms(!). Multiplying the delta time
	// with the conveyor speed (in m/s) results in the predicted displacement
	// in mm. The potato is assumed to haven't moved in x direction.
	deltaT = unCurTime - unLastSeenTime_ms; 

	// Store the resulting position in this struct's members.
	nPredictedPos_mm_X = nLastSeenPos_mm_X;
	nPredictedPos_mm_Y = nLastSeenPos_mm_Y + ( (fp16ConveyorSpeed * deltaT) >> 16);
}
					
// *************************************************************************
