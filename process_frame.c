/*! @file process_frame.c
 * @brief Contains the actual algorithm and calculations.
 */

#include "template.h"
// #include "framework_extensions.h"

#define SEGMENT_POOL_COUNT 64
#define OBJECT_POOL_COUNT 64

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#define DEBUG

#ifdef DEBUG
	#define printMark() printf("%s: Line %d\n", __func__, __LINE__)
	#define m printf("%s: Line %d\n", __func__, __LINE__);
	#define p(name) printf("%s: %ld\n", # name, name);
#else
	#define printMark()
	#define m
	#define p(name)
#endif

struct aSegment {
	struct segment {
		uint16 begin, end;
		struct object * pObject;
	} segments[SEGMENT_POOL_COUNT];
	
	uint8 numSegments;
} segmentPools[2];

struct aObject {
	struct object {
		uint16 left, right, top, bottom;
		uint32 posWghtX, posWghtY;
		uint32 weight;
		struct object * pPrev, * pNext;
	} objects[OBJECT_POOL_COUNT];
	
	struct object * pFirstInactive, * pFirstActive;
} object_pool;

/* void object_pool_dump(struct aObject const * const pPool)
{
	uint8 i;
	
	printf("pFirstInactive: %d\n", pPool->pFirstInactive - (pPool->objects));
	printf("pFirstActive: %d\n", pPool->pFirstActive - (pPool->objects));
	
	for (i = 0; i < OBJECT_POOL_COUNT; i += 1)
	{
		printf("%u: %d, %d\n", i,
			pPool->objects[i].pPrev - (pPool->objects),
			pPool->objects[i].pNext - (pPool->objects));
	}
} */

/* initializes the object pool to a state where all objects are inactive */
void object_pool_init (struct aObject * const pPool)
{
	uint8 i;
	
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
	if (pObj->pPrev == NULL)
	{ /* The object is at the begining of the inactive list. */
		pPool->pFirstInactive = pObj->pNext;
	}
	else
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
	if (pObj->pNext != NULL)
	{
		pObj->pNext->pPrev = pObj;
	}
	pPool->pFirstActive = pObj;
}

void object_pool_deactivate (struct aObject * const pPool, struct object * const pObj)
{
//	printf("pObj: %d\n", pObj - pPool->objects);
	/* First, we remove the object from the active list. */
	if (pObj->pPrev == NULL)
	{
		pPool->pFirstActive = pObj->pNext;
	}
	else
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
	if (pObj->pNext != NULL)
	{
		pObj->pNext->pPrev = pObj;
	}
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
	
	for (pSegs->numSegments = 0; pSegs->numSegments < SEGMENT_POOL_COUNT;
		pSegs->numSegments += 1)
	{	
		for (; i < width && pImg[i] != value; i += 1);
		pSegs->segments[pSegs->numSegments].begin = i;
		if (i == width)
		{
			break;
		}
		for (; i < width && pImg[i] == value; i += 1);
		/* we ended a segment, possibly at the end of the line */
		pSegs->segments[pSegs->numSegments].end = i;
		pSegs->segments[pSegs->numSegments].pObject = NULL;
		if (i == width)
		{
			pSegs->numSegments += 1;
			break;
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
	struct object * merge_objects(struct object * pObj1, struct object * pObj2, struct aSegment * pSegs1, struct aSegment * pSegs2)
	{
	//	printf("obj1: %u, obj2: %u\n", pObj1 - object_pool.objects, pObj2 - object_pool.objects);
		
		if (pObj1 == NULL)
		{
m			return pObj2;
		}
		else if (pObj2 == NULL)
		{
m			return pObj1;
		} 
		else if (pObj1 != pObj2)
		{ /* If the objects are not the same the second is merged into the first one. */
			uint8 i;
			
			pObj1->left = min(pObj1->left, pObj2->left);
			pObj1->right = max(pObj1->right, pObj2->right);
			pObj1->weight = pObj1->weight + pObj2->weight;
			pObj1->top = min(pObj1->top, pObj2->top);
			pObj1->bottom = max(pObj1->bottom, pObj2->bottom);
			pObj1->posWghtX = pObj1->posWghtX + pObj2->posWghtX;
			pObj1->posWghtY = pObj1->posWghtY + pObj2->posWghtY;
			
			/* Relabel all segments of the merged object. */
			for (i = 0; i < pSegs1->numSegments; i += 1)
			{
				if (pSegs1->segments[i].pObject == pObj2)
				{
					pSegs1->segments[i].pObject = pObj1;
				}
			}
			for (i = 0; i < pSegs2->numSegments; i += 1)
			{
				if (pSegs2->segments[i].pObject == pObj2)
				{
					pSegs2->segments[i].pObject = pObj1;
				}
			}
			
		//	pObj2->pObjectMergedTo = pObj2;
			object_pool_deactivate (&object_pool, pObj2);
		}
		
		return pObj1;
	}
	
	uint16 i;
	uint16 iLast, iCurrent;
	struct aSegment * segmentsLast = segmentPools, * segmentsCurrent = segmentPools + 1;
	
	/* This marks all objects as inactive. */
	
	object_pool_init(&object_pool);
	
//	findSegments(pImg, width, value, segmentsCurrent);
	segmentsCurrent->numSegments = 0;
	
	for (i = 0; i < height; i += 1) /* this loops over every line, starting from the second */
	{ /* both segmentsLast and segmentsCurrent point to a valid aSegment instance */
		struct object * obj = NULL; /* This holds the object for the last segment processed. */
		
		/* swap the pointers to the last and the current segment array */
		struct aSegment * segmentsTemp = segmentsLast;
		segmentsLast = segmentsCurrent;
		findSegments(pImg + i * width, width, value, segmentsCurrent = segmentsTemp);
		
		iLast = iCurrent = 0;
		
		while (iLast < segmentsLast->numSegments
			|| iCurrent < segmentsCurrent->numSegments) /* this loops over all segments of the last and current line */
		{ /* both segmentsLast and segmentsCurrent point to a valid aSegment instance, but iLast and iCurrent may point past the end of the array. */
			/* First we check, wether we moved on the current or on the last line the last step. */
			
			if (iCurrent < segmentsCurrent->numSegments)
			{ /* There are segments on the current line. */
				if (obj == NULL)
				{ /* This means that we either moved on the current line or that we are on the first segment. So we create an object for the lower line. */
					obj = create_object_for_segment(i, segmentsCurrent->segments + iCurrent);
				}
				
				/* So we check whether we also have segments on the last line. */
				if (iLast < segmentsLast->numSegments)
				{ /* We do have segments on the last and the current line so we check if they overlap. */
					if (segmentsLast->segments[iLast].begin
							< segmentsCurrent->segments[iCurrent].end
						&& segmentsCurrent->segments[iCurrent].begin
							< segmentsLast->segments[iLast].end)
					{ /* They do overlap so we merge the segment from the current line into the object from the segment from the last. */
						obj = merge_objects(
							segmentsLast->segments[iLast].pObject, obj, segmentsLast, segmentsCurrent);
						
						/* We need to check which segment ends first. */
						if (segmentsLast->segments[iLast].end
							< segmentsCurrent->segments[iCurrent].end)
						{
							iLast += 1;
						}
						else
						{
							obj = NULL;
							iCurrent += 1;
						}
					}
					else
					{ /* They do not overlap so we just have to create a new object for the segment of the current line. */
						/* We need to check which segment ends first. */
						if (segmentsLast->segments[iLast].end
							< segmentsCurrent->segments[iCurrent].end)
						{ /* The segment on the last line ends first, so we just skip the segment from the last line. */
							iLast += 1;
						}
						else
						{ /* The segment on the current line ends first. So we will have to generate a new object for the next segment on the current line. */
							obj = NULL;
							iCurrent += 1;
						}
					}
				}
				else
				{ /* We only have segments on the current line. This menas that we have to generate a new object for the next segment. */
					obj = NULL;
					iCurrent += 1;
				}
			}
			else
			{ /* There are no segments on the current line left so we just skip the ones from the last line. */
				iLast += 1;
			}
		}
	}
	
	return object_pool_getActive(&object_pool);
}

void drawRectangle(uint8 * const pImg, uint16 const width, uint16 const left, uint16 const right, uint16 const top, uint16 const bottom, uint8 const color[3])
{
	uint16 i;
	
	/* Draw the horizontal lines. */
	for (i = left; i < right; i += 1)
	{
		pImg[(width * top + i) * 3] = color[0];
		pImg[(width * top + i) * 3 + 1] = color[1];
		pImg[(width * top + i) * 3 + 2] = color[2];
		
		pImg[(width * (bottom - 1) + i) * 3] = color[0];
		pImg[(width * (bottom - 1) + i) * 3 + 1] = color[1];
		pImg[(width * (bottom - 1) + i) * 3 + 2] = color[2];
	}
	
	/* Draw the vertical lines. */
	for (i = top; i < bottom; i += 1)
	{
		pImg[(width * i + left) * 3] = color[0];
		pImg[(width * i + left) * 3 + 1] = color[1];
		pImg[(width * i + left) * 3 + 2] = color[2];
		
		pImg[(width * i + right - 1) * 3] = color[0];
		pImg[(width * i + right - 1) * 3 + 1] = color[1];
		pImg[(width * i + right - 1) * 3 + 2] = color[2];
	}
}

void ProcessFrame(uint8 const * const pRawImg)
{
	OSC_ERR err;
	enum EnBayerOrder enBayerOrder;
	uint16 row, col, greyCol, greyRow, greyWidth, greyHeight;
	uint16 width = OSC_CAM_MAX_IMAGE_WIDTH, height = OSC_CAM_MAX_IMAGE_HEIGHT;
	uint32 pos;
	uint8 const threshold = 40;
	
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
	
	greyWidth = width / 2;
	greyHeight = height / 2;
	
	/* masks parts of the image that contain an objcet */
	applyThreshold(pGrayImg, greyWidth, greyHeight, threshold, TRUE, FALSE);
	
	{
		struct object * objs = findObjects(pGrayImg, greyWidth, greyHeight, 0);
		
		for (; objs != NULL; objs = objs->pNext)
		{
			uint8 green[3] = {0, ~0, 0};
			
			printf("Left: %lu, Right: %lu, Top: %lu, Bottom: %lu, Weight: %lu\n", objs->left, objs->right, objs->top, objs->bottom, objs->weight);
			
			drawRectangle(data.u8ResultImage, width, objs->left * 2, objs->right * 2, objs->top * 2, objs->bottom * 2, green);
		}
		
		printf("\n");
	}
	
	for (greyRow = 0; greyRow < greyHeight; greyRow += 1)
	{
		row = greyRow * 2;
		for (greyCol = 0; greyCol < greyWidth; greyCol += 1)
		{
			uint8 grey;
			col = greyCol * 2;
			
			pos = (row * width + col) * 3;
			
			grey = pGrayImg[greyRow * greyWidth + greyCol];
			
			if (grey == 0)
			{
				data.u8ResultImage[pos + 0] = 0;
				data.u8ResultImage[pos + 1] = 0;
				data.u8ResultImage[pos + 2] = ~0;
			}
		}
	}
}

void processFrame_init() {
//	segmentPools_init ();
}
