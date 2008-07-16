/*! @file process_frame.c
 * @brief Contains the actual algorithm and calculations.
 */

#include "template.h"
// #include "framework_extensions.h"

#define SEGMENT_POOL_COUNT 64
#define OBJECT_POOL_COUNT 64

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#define printMark() printf("%s: Line %d\n", __func__, __LINE__)
#define m printf("%s: Line %d\n", __func__, __LINE__);
#define p(name) printf("%s: %ld\n", # name, name);

void debugMark(uint16 const num)
{
	printf("<<%u>>", num);
	fflush(stdout);
}

struct aSegment {
	struct segment {
		uint16 begin, end;
		struct object * pObject;
	} segments[SEGMENT_POOL_COUNT];
	
	uint8 numSegments;
} segment_pools[2];

struct aObject {
	struct object {
	//	uint16 left, right, top, bottom;
	//	uint32 posWghtX, posWghtY;
	//	uint32 weight;
		unsigned short int left, right, top, bottom;
		unsigned long int posWghtX, posWghtY;
		unsigned long int weight;
	//	struct object * pObjectMergedTo;
		struct object * pPrev, * pNext;
	} objects[OBJECT_POOL_COUNT];
	
	struct object * pFirstInactive, * pFirstActive;
} object_pool;

/* initializes the object pool to a state where all objects are inactive */
void object_pool_init (struct aObject * const pPool)
{
	int i;
	
	for (i = 1; i < OBJECT_POOL_COUNT; i += 1)
	{
		pPool->objects[i - 1].pNext = pPool->objects + i;
		pPool->objects[i].pPrev = pPool->objects + i - 1;
	}
	
	pPool->pFirstInactive = pPool->objects;
	pPool->pFirstActive = NULL;
	
	pPool->objects[0].pPrev = NULL;
	pPool->objects[OBJECT_POOL_COUNT - 1].pNext = NULL;
}

struct object * object_pool_getInactive (struct aObject const * const pPool)
{
	return pPool->pFirstInactive;
}

struct object * object_pool_getActive (struct aObject const * const pPool)
{
	return pPool->pFirstActive;
}

void object_pool_activate (struct aObject * const pPool, struct object * const pObj)
{
	/* First, we remove the object from the inactive list. */
	if (pObj->pPrev != NULL)
	{
		pObj->pPrev->pNext = pObj->pNext;
	}
	if (pObj->pNext != NULL)
	{
		pObj->pNext->pPrev = pObj->pPrev;
	}
	
	/* And then put it into the active list. */
	pObj->pPrev = NULL;
	pObj->pNext = pPool->pFirstActive;
	pPool->pFirstActive = pObj;
}

void object_pool_deactivate (struct aObject * const pPool, struct object * const pObj)
{
	/* First, we remove the object from the active list. */
	if (pObj->pPrev != NULL)
	{
		pObj->pPrev->pNext = pObj->pNext;
	}
	if (pObj->pNext != NULL)
	{
		pObj->pNext->pPrev = pObj->pPrev;
	}
	
	/* And then put it into the inactive list. */
	pObj->pPrev = NULL;
	pObj->pNext = pPool->pFirstInactive;
	pPool->pFirstInactive = pObj;
}

/*!
 * @brief Mask images with a threshold.
 * 
 * Masks pixels of an image according to a threshold. Pixels darker or lighter than the threshold are masked with the darkes or brightest value respectively
 * 
 * ! Only even widths and heights are supported !
 * 
 * The output picture has 8 bit greyscale cells with halve the with and height of the original image.
 * 
 * @param pImg Pointer to the raw input picture of size width x height.
 * @param width Width of the input and output image.
 * @param height Height of the input and output image.
 * @param threshold The threshold uf the threshold to use. True menas that values greater than threshold are masked to the maximum value (255). False means that values smaller than the threshod are masked to the minimum value (0).
 * @param maskLower If set, pixels with a value smaller than the threshold are masked with the smalles value (0).
 * @param maskUpper If set, pixels with a value of threshold or greater are masked with the largest value (255).
 */


void applyThreshold(uint8 * const pImg, uint16 const width, uint16 const height, uint8 const threshold, bool const maskLower, bool const maskUpper)
{
	uint32 pos;
	
	if (maskLower)
	{
		if (maskUpper)
		{
			for(pos = 0; pos < width * height; pos += 1)
			{
				if (pImg[pos] < threshold)
				{
					pImg[pos] = 0;
				}
				else
				{
					pImg[pos] = ~0;
				}
			}
		}
		else
		{
			for(pos = 0; pos < width * height; pos += 1)
			{
				if (pImg[pos] < threshold)
				{
					pImg[pos] = 0;
				}
			}
		}
	}
	else
	{
		if (maskUpper)
		{
			for(pos = 0; pos < width * height; pos += 1)
			{
				if (pImg[pos] >= threshold)
				{
					pImg[pos] = ~0;
				}
			}
		}
	}
}


/* void applyThreshold(uint8 * const pImg, uint16 const width, uint16 const height, uint8 const threshold, bool const maskLower, bool const maskUpper) {
	uint32 pos;
	
	if (maskLower)
		if (maskUpper)
			for(pos = 0; pos < width * height; pos += 1)
				if (pImg[pos] < threshold)
					pImg[pos] = 0;
				else
					pImg[pos] = ~0;
		else
			for(pos = 0; pos < width * height; pos += 1)
				if (pImg[pos] < threshold)
					pImg[pos] = 0;
	else
		if (maskUpper)
			for(pos = 0; pos < width * height; pos += 1)
				if (pImg[pos] >= threshold)
					pImg[pos] = ~0;
} */

/*!
 * @brief Find segments in the first line of a picture.
 *
 * @param pImg The image to find the segments in.
 * @param width The width of the image.
 * @param value The value to be considered part of a segment.
 * @param pSegs A pointer to the segment array.
 */
void findSegments(uint8 const * const pImg, uint16 const width, uint8 const value, struct aSegment * const pSegs)
{
	uint16 i = 0;
	
m	for (pSegs->numSegments = 0; pSegs->numSegments < SEGMENT_POOL_COUNT; pSegs->numSegments += 1)
	{	
		for (; i < width && pImg[i] != value; i += 1);
		pSegs->segments[pSegs->numSegments].begin = i;
m		p(i)
		for (; i < width && pImg[i] == value; i += 1);
		/* we ended a segment, possibly at the end of the line */
		pSegs->segments[pSegs->numSegments].end = i;
		pSegs->segments[pSegs->numSegments].pObject = NULL;
m		p(i)
		if (i == width)
		{
m			break;
		}
	}
	/* we hit the end of the line or ran out of segments */
}

/*!
 * @brief Find objects in a picture.
 *
 * @param pImg The image to find the segments in.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param value The value to be considered part of an object.
 * @return A pointer to the object array.
 */
struct object * findObjects(uint8 const * const pImg, uint16 const width, uint16 const height, uint8 const value) {
	struct object * create_object_for_segment(uint16 line, struct segment * pSeg)
	{
		struct object * obj = object_pool_getInactive(&object_pool);
		
		if (obj != NULL)
		{
			obj->left = pSeg->begin;
			obj->right = pSeg->end;
			obj->weight = obj->right - obj->left;
			obj->top = line;
			obj->bottom = line + 1;
			obj->posWghtX = obj->weight * (obj->left + obj->right) / 2;
			obj->posWghtY = obj->weight * line;
			
			object_pool_activate(&object_pool, obj);
			pSeg->pObject = obj;
		}
		
		return obj;
	}
	
	/* Merges two objects and re-labels the segments given. */
	struct object * merge_objects (struct object * pObj1, struct object * pObj2, struct aSegment * pSegs)
	{
		uint8 i;
		
		if (pObj1 == NULL)
		{
			return pObj2;
		}
		else if (pObj2 == NULL)
		{
			return pObj1;
		} 
		else if (pObj1 != pObj2)
		{ /* If the objects are not the same the second is merged into the first one. */
			pObj1->left = min(pObj1->left, pObj2->left);
			pObj1->right = max(pObj1->right, pObj2->right);
			pObj1->weight = pObj1->weight + pObj2->weight;
			pObj1->top = min(pObj1->top, pObj2->top);
			pObj1->bottom = max(pObj1->bottom, pObj2->bottom);
			pObj1->posWghtX = pObj1->posWghtX + pObj2->posWghtX;
			pObj1->posWghtY = pObj1->posWghtY + pObj2->posWghtY;
			
			for (i = 0; i < pSegs->numSegments; i += 1)
			{
				if (pSegs->segments[i].pObject == pObj2)
				{
					pSegs->segments[i].pObject = pObj1;
				}
			}
			
		//	pObj2->pObjectMergedTo = pObj2;
			object_pool_deactivate (&object_pool, pObj2);
		}
		
		return pObj1;
	}
	
	uint16 i;
	uint16 iLast, iCurrent;
	struct aSegment * segmentsLast = segment_pools, * segmentsCurrent = segment_pools + 1;
	struct object * obj; /* This holds the object for the last segment processed. */
	
	/* This marks all objects as inactive. */
	
m	object_pool_init(&object_pool);
	
//	findSegments(pImg, width, value, segmentsCurrent);
	segmentsCurrent->numSegments = 0;
	
	for (i = 0; i < height; i += 1) /* this loops over every line, starting from the second */
	{ /* both segmentsLast and segmentsCurrent point to a valid aSegment instance */
		/* swap the pointers to the last and the current segment array */
		struct aSegment * segmentsTemp = segmentsLast;
		segmentsLast = segmentsCurrent;
		findSegments(pImg + i * width, width, value, segmentsCurrent = segmentsTemp);
		
p(i)	for (iCurrent = 0; iCurrent < segmentsCurrent->numSegments; iCurrent += 1)
		{
			printf("begin: %u, end: %u\n",
				segmentsCurrent->segments[iCurrent].begin,
				segmentsCurrent->segments[iCurrent].end);
		}
		
		iLast = iCurrent = 0;
		
		while (iLast < segmentsLast->numSegments
			|| iCurrent < segmentsCurrent->numSegments) /* this loops over all segments of the last and current line */
		{ /* both segmentsLast and segmentsCurrent point to a valid aSegment instance, but iLast and iCurrent may point past the end of the array. */
m			if (iLast < segmentsLast->numSegments)
			{ /* We have segments on the last line so we check whether we have segments on the current line. */
m				if (iCurrent < segmentsCurrent->numSegments)
				{ /* We do have segments on the last and the current line. */
m					if (segmentsLast->segments[iLast].begin
							< segmentsCurrent->segments[iCurrent].end
						&& segmentsCurrent->segments[iCurrent].begin
							< segmentsLast->segments[iLast].end)
					{ /* They do overlap so we merge the segment from the current line into the object from the segment from the last. */
m						obj = create_object_for_segment(i,
							segmentsCurrent->segments + iCurrent);
						obj = merge_objects(obj,segmentsLast->segments[iLast]
							.pObject, segmentsCurrent);
						
						/* We need to check which segment ends first. */
m						if (segmentsLast->segments[iLast].end
							< segmentsCurrent->segments[iCurrent].end)
						{
							iLast += 1;
						}
						else
						{
							iCurrent += 1;
						}
					}
					else
					{ /* They do not overlap so we just have to create a new object for the segment of the current line. */
m						obj = create_object_for_segment(i, segmentsCurrent
							->segments + iCurrent);
						
						iLast += 1;
						iCurrent += 1;
					}
				}
				else
				{
					iLast += 1;
				}
			}
			else
			{ /* We have no more segments on the last line so we need to create objects for all segments on the current line. */
m				for (; iCurrent < segmentsCurrent->numSegments;
					iCurrent += 1)
				{
m					create_object_for_segment(i, segmentsCurrent->segments + iCurrent);
				}
			}
		}
	}
	
	return object_pool_getActive(&object_pool);
}

void ProcessFrame(uint8 const * const pRawImg)
{
	OSC_ERR err;
	enum EnBayerOrder enBayerOrder;
	uint16 row, col, width, height, greyCol, greyRow, greyWidth, greyHeight;
	uint32 pos;
	uint8 const threshold = 30;
	
	/*! @brief Grayscale image with half the with an height */
	static uint8 pGrayImg[OSC_CAM_MAX_IMAGE_WIDTH * OSC_CAM_MAX_IMAGE_HEIGHT / 4];
//	int i;
	
	err = OscCamGetBayerOrder(&enBayerOrder, 0, 0);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Error getting bayer order! (%d)\n", __func__, err);
		return;
	}
	
	/* Use a framework function to debayer the image. */
	err = OscVisDebayer(pRawImg, OSC_CAM_MAX_IMAGE_WIDTH, OSC_CAM_MAX_IMAGE_HEIGHT, enBayerOrder, data.u8ResultImage);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Error debayering image! (%d)\n", __func__, err);
		return;
	}
	
	err = OscVisDebayerGrayscaleHalfSize (pRawImg, OSC_CAM_MAX_IMAGE_WIDTH, OSC_CAM_MAX_IMAGE_HEIGHT, enBayerOrder, pGrayImg);
	if(err != SUCCESS)	
	{
		OscLog(ERROR, "%s: Error debayering image! (%d)\n", __func__, err);
		return;
	}
	/* Add your code here */
	
	width = OSC_CAM_MAX_IMAGE_WIDTH;
	height = OSC_CAM_MAX_IMAGE_HEIGHT;
	greyWidth = width / 2;
	greyHeight = height / 2;
	
	/* masks parts of the image that contain an objcet */
	applyThreshold(pGrayImg, greyWidth, greyHeight, threshold, TRUE, FALSE);
	
m	{
		struct object * objs = findObjects(pGrayImg, greyWidth, greyHeight, threshold);
		
		for (; objs != NULL; objs = objs->pNext)
		{
			printf("Left: %lu, Right: %lu, Top: %lu, Bottom: %lu, Weight: %lu\n", objs->left, objs->right, objs->top, objs->bottom, objs->weight);
		}
		
		printf("\n");
	}
	
m	for (greyRow = 0; greyRow < greyHeight; greyRow += 1)
	{
		row = greyRow * 2;
		for (greyCol = 0; greyCol < greyWidth; greyCol += 1)
		{
			uint8 grey;
			col = greyCol * 2;
			
			pos = (row * width + col) * 3;
			
			grey = pGrayImg[greyRow * greyWidth + greyCol];
			
		/*	data.u8ResultImage[pos + 0] = grey;
			data.u8ResultImage[pos + 1] = grey;
			data.u8ResultImage[pos + 2] = grey;
			data.u8ResultImage[pos + 3] = grey;
			data.u8ResultImage[pos + 4] = grey;
			data.u8ResultImage[pos + 5] = grey;
		*/	
			if (grey == 0)
			{
				data.u8ResultImage[pos + 0] = 0;
				data.u8ResultImage[pos + 1] = 0;
				data.u8ResultImage[pos + 2] = ~0;
			}
			
		/*	if ((greyRow == greyHeight - 1) && (greyCol == greyWidth - 1))
			{
				printf ("%lu\n", pos);
				data.u8ResultImage[pos + 0] = 0;
				data.u8ResultImage[pos + 1] = 0;
				data.u8ResultImage[pos + 2] = ~0;
			}
		*/
		}
	}
	
/*	for (i = 0; i < 3 * OSC_CAM_MAX_IMAGE_WIDTH * OSC_CAM_MAX_IMAGE_HEIGHT; i += 1)
	{
		data.u8ResultImageColor[i] ^= ~0;
	}
*/
}

void processFrame_init() {
//	segment_pools_init ();
}
