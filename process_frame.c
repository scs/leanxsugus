/*! @file process_frame.c
 * @brief Contains the actual algorithm and calculations.
 */

#include "leanxsugus.h"
// #include "framework_extensions.h"

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#define length(a) ((sizeof (a)) / sizeof *(a))

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

typedef uint16 t_index;

typedef struct {
	struct segment {
		t_index begin, end;
		struct object * pObject;
	} segments[64];
	
	t_index numSegments;
} s_segmentArray;

typedef struct {
	struct object {
		t_index left, right, top, bottom;
		uint32 posWghtX, posWghtY;
		uint32 weight;
		struct object * pPrev, * pNext;
	} objects[500];
	
	struct object * pFirst[2];
} s_objectPool;

/* void objectPool_dump(struct aObject const * const pPool)
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
void objectPool_init (s_objectPool * const pPool)
{
	t_index i;
	
	for (i = 1; i < length (pPool->objects); i += 1)
	{
		pPool->objects[i - 1].pNext = pPool->objects + i;
		pPool->objects[i].pPrev = pPool->objects + i - 1;
	}
	
	pPool->pFirst[0] = pPool->objects;
	for (i = 1; i < length (pPool->pFirst); i += 1)
	{
		pPool->pFirst[i] = NULL;
	}
	
	pPool->objects[0].pPrev = NULL;
	pPool->objects[length (pPool->objects) - 1].pNext = NULL;
}

void objectPool_move (s_objectPool * const pPool, struct object * const pObj, t_index from, t_index to)
{
	/* First, we remove the object from the from list. */
	if (pObj->pPrev == NULL)
	{ /* The object is at the begining of the inactive list. */
		pPool->pFirst[from] = pObj->pNext;
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
	pObj->pNext = pPool->pFirst[to];
	if (pObj->pNext != NULL)
	{
		pObj->pNext->pPrev = pObj;
	}
	pPool->pFirst[to] = pObj;
}

/*!
 * @brief Mask images with a threshold.
 * 
 * Masks pixels of an image according to a threshold. Pixels darker or lighter than the threshold are masked with the darkes or brightest value respectively
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
/* This function does not follow the coding conventions as they blow up this code section to three times its size. it would be much harder to read, believe me. For reference, the conforming version is appended at the bottom. */
void applyThreshold(uint8 * const pImg, uint16 const width, uint16 const height, uint8 const threshold, bool const maskLower, bool const maskUpper) {
	uint32 pos;
	
	if (maskLower)
		if (maskUpper)
			for (pos = 0; pos < width * height; pos += 1)
				if (pImg[pos] < threshold)
					pImg[pos] = 0;
				else
					pImg[pos] = ~0;
		else
			for (pos = 0; pos < width * height; pos += 1)
				if (pImg[pos] < threshold)
					pImg[pos] = 0;
				else;
	else
		if (maskUpper)
			for (pos = 0; pos < width * height; pos += 1)
				if (pImg[pos] >= threshold)
					pImg[pos] = ~0;
}

/* void applyThreshold(uint8 * const pImg, uint16 const width, uint16 const height, uint8 const threshold, bool const maskLower, bool const maskUpper)
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
} */

/*!
 * @brief Find segments in the first line of a picture.
 *
 * @param pImg The image to find the segments in.
 * @param width The width of the image.
 * @param value The value to be considered part of a segment.
 * @param pSegs A pointer to the segment array.
 */
void findSegments(uint8 const * const pImg, uint16 const width, uint8 const value, s_segmentArray * const pSegArr)
{
	uint16 i = 0;
	
	for (pSegArr->numSegments = 0;
		pSegArr->numSegments < length (pSegArr->segments);
		pSegArr->numSegments += 1)
	{	
		while (i < width && pImg[i] != value)
			i += 1;
		pSegArr->segments[pSegArr->numSegments].begin = i;
		if (i == width)
			break;
		
		while (i < width && pImg[i] == value)
			i += 1;
		/* we ended a segment, possibly at the end of the line */
		pSegArr->segments[pSegArr->numSegments].end = i;
		pSegArr->segments[pSegArr->numSegments].pObject = NULL;
		if (i == width)
		{
			pSegArr->numSegments += 1;
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
	struct object * createObjectForSegment(t_index line, struct segment * pSeg, s_objectPool * pObjPool)
	{
		struct object * obj = pObjPool->pFirst[0];
		
		if (obj != NULL)
		{
			obj->left = pSeg->begin;
			obj->right = pSeg->end;
			obj->weight = obj->right - obj->left;
			obj->top = line;
			obj->bottom = line + 1;
			obj->posWghtX = obj->weight * (obj->left + obj->right) / 2;
			obj->posWghtY = obj->weight * line;
			
			objectPool_move(pObjPool, obj, 0, 1);
			pSeg->pObject = obj;
		}
		
		return obj;
	}
	
	/* Merges two objects and re-labels the segments given. */
	struct object * mergeObjects(struct object * pObj1, struct object * pObj2, s_objectPool * pObjPool, s_segmentArray * pSegArr1, s_segmentArray * pSegArr2)
	{
	//	printf("obj1: %u, obj2: %u\n", pObj1 - objectPool.objects, pObj2 - objectPool.objects);
		
		if (pObj1 == NULL)
			return pObj2;
		else if (pObj2 == NULL)
			return pObj1;
		else if (pObj1 != pObj2)
		{ /* If the objects are not the same the second is merged into the first one. */
			t_index i;
			
			pObj1->left = min(pObj1->left, pObj2->left);
			pObj1->right = max(pObj1->right, pObj2->right);
			pObj1->weight = pObj1->weight + pObj2->weight;
			pObj1->top = min(pObj1->top, pObj2->top);
			pObj1->bottom = max(pObj1->bottom, pObj2->bottom);
			pObj1->posWghtX = pObj1->posWghtX + pObj2->posWghtX;
			pObj1->posWghtY = pObj1->posWghtY + pObj2->posWghtY;
			
			/* Relabel all segments of the merged object. */
			for (i = 0; i < pSegArr1->numSegments; i += 1)
				if (pSegArr1->segments[i].pObject == pObj2)
					pSegArr1->segments[i].pObject = pObj1;
			
			for (i = 0; i < pSegArr2->numSegments; i += 1)
				if (pSegArr2->segments[i].pObject == pObj2)
					pSegArr2->segments[i].pObject = pObj1;
			
		//	pObj2->pObjectMergedTo = pObj2;
			objectPool_move (pObjPool, pObj2, 1, 0);
		}
		
		return pObj1;
	}
	
	t_index i;
	t_index iLast, iCurrent;
	
	static s_objectPool objPool;
	static s_segmentArray segArrs[2];
	s_segmentArray * segsLast = segArrs, * segsCurr = segArrs + 1;
	
	/* This marks all objects as inactive. */
	objectPool_init(&objPool);
	
//	findSegments(pImg, width, value, segsCurr);
	segsCurr->numSegments = 0;
	
	for (i = 0; i < height; i += 1) /* this loops over every line, starting from the second */
	{ /* both segsLast and segsCurr point to a valid aSegment instance */
		struct object * obj = NULL; /* This holds the object for the last segment processed. */
		
		/* swap the pointers to the last and the current segment array */
		s_segmentArray * segmentsTemp = segsLast;
		segsLast = segsCurr;
		segsCurr = segmentsTemp;
		findSegments(pImg + i * width, width, value, segsCurr);
		
		iLast = iCurrent = 0;
		
		while (iLast < segsLast->numSegments
			|| iCurrent < segsCurr->numSegments) /* this loops over all segments of the last and current line */
		{ /* both segsLast and segsCurr point to a valid aSegment instance, but iLast and iCurrent may point past the end of the array. */
			/* First we check, wether we moved on the current or on the last line the last step. */
			
			if (iCurrent < segsCurr->numSegments)
			{ /* There are segments on the current line. */
				if (obj == NULL)
				{ /* This means that we either moved on the current line or that we are on the first segment. So we create an object for the lower line. */
					obj = createObjectForSegment(i, segsCurr->segments + iCurrent, &objPool);
				}
				
				/* So we check whether we also have segments on the last line. */
				if (iLast < segsLast->numSegments)
				{ /* We do have segments on the last and the current line so we check if they overlap. */
					if (segsLast->segments[iLast].begin
							< segsCurr->segments[iCurrent].end
						&& segsCurr->segments[iCurrent].begin
							< segsLast->segments[iLast].end)
					{ /* They do overlap so we merge the segment from the current line into the object from the segment from the last. */
						obj = mergeObjects(segsLast->segments[iLast].pObject, obj, &objPool, segsLast, segsCurr);
						
						/* We need to check which segment ends first. */
						if (segsLast->segments[iLast].end
							< segsCurr->segments[iCurrent].end)
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
						if (segsLast->segments[iLast].end
							< segsCurr->segments[iCurrent].end)
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
	
	return objPool.pFirst[1];
}

void drawRectangle(uint8 * const pImg, t_index const width, t_index const left, t_index const right, t_index const top, t_index const bottom, uint8 const color[3])
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
	t_index row, col, greyCol, greyRow, greyWidth, greyHeight;
	t_index width = OSC_CAM_MAX_IMAGE_WIDTH, height = OSC_CAM_MAX_IMAGE_HEIGHT;
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
	err = OscVisDebayer(pRawImg, width, height, enBayerOrder, data.u8ResultImage);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Error debayering image! (%d)\n", __func__, err);
		return;
	}
	
	err = OscVisDebayerGrayscaleHalfSize (pRawImg, width, height, enBayerOrder, pGrayImg);
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
			uint32 pos;
			
			col = greyCol * 2;
			pos = (row * width + col) * 3;
			grey = pGrayImg[greyRow * greyWidth + greyCol];
			
		/*	if (grey == 0)
			{
				data.u8ResultImage[pos + 0] = 0;
				data.u8ResultImage[pos + 1] = 0;
				data.u8ResultImage[pos + 2] = ~0;
			} */
		}
	}
}

void processFrame_init() {
//	segmentArrays_init ();
}
