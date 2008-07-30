/*! @file process_frame.c
 * @brief Contains the actual algorithm and calculations.
 */

#include "modbus.h"
#include "process.h"

#if defined(OSC_HOST)
#define IMG_FILENAME "/var/www/image.bmp"
#else
#define IMG_FILENAME "/home/httpd/image.bmp"
#endif

#define widthGrey (WIDTH_CAPTURE / 2)
#define heightGrey (HEIGHT_CAPTURE / 2)

#define m printf("%s: Line %d\n", __func__, __LINE__);
//#define m

struct {
	enum EnBayerOrder enBayerOrder;
	uint8 imgColor[3 * WIDTH_CAPTURE * HEIGHT_CAPTURE];
	/*! @brief Greyscale image with half the with an height */
	uint8 imgGrey[widthGrey * heightGrey];
} data;

typedef enum {
	e_classification_sugusRed,
	e_classification_sugusGreen,
	e_classification_sugusOrange,
	e_classification_sugusYellow,
	e_classification_otherColor,
	e_classification_tooSmall,
	e_classification_tooNearToBorder
} e_classification;

typedef struct {
	uint8 blue, green, red;
} s_color;

typedef struct {
	struct segment {
		t_index begin, end;
		struct object * pObject;
	} segments[1000];
	
	t_index numSegments;
} s_segmentArray;

typedef struct {
	struct object {
		uint16 left, right, top, bottom;
		uint32 posWghtX, posWghtY;
		uint32 weight;
		s_color color;
		e_classification classification;
		struct object * pPrev, * pNext;
	} objects[1000];
	
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
void applyThreshold(uint8 const threshold, bool const maskLower, bool const maskUpper) {
	uint32 pos;
	
	if (maskLower)
		if (maskUpper)
			for (pos = 0; pos < widthGrey * heightGrey; pos += 1)
				if (data.imgGrey[pos] < threshold)
					data.imgGrey[pos] = 0;
				else
					data.imgGrey[pos] = ~0;
		else
			for (pos = 0; pos < widthGrey * heightGrey; pos += 1)
				if (data.imgGrey[pos] < threshold)
					data.imgGrey[pos] = 0;
				else;
	else
		if (maskUpper)
			for (pos = 0; pos < widthGrey * heightGrey; pos += 1)
				if (data.imgGrey[pos] >= threshold)
					data.imgGrey[pos] = ~0;
}

/*!
 * @brief Find segments in the first line of a picture.
 *
 * @param pImg The image to find the segments in.
 * @param width The width of the image.
 * @param value The value to be considered part of a segment.
 * @param pSegs A pointer to the segment array.
 */
void findSegments(uint8 const * const pImg, uint8 const value, s_segmentArray * const pSegArr)
{
	uint16 i = 0;
	
	for (pSegArr->numSegments = 0;
		pSegArr->numSegments < length (pSegArr->segments);
		pSegArr->numSegments += 1)
	{	
		while (i < widthGrey && pImg[i] != value)
			i += 1;
		pSegArr->segments[pSegArr->numSegments].begin = i;
		if (i == widthGrey)
			break;
		
		while (i < widthGrey && pImg[i] == value)
			i += 1;
		/* we ended a segment, possibly at the end of the line */
		pSegArr->segments[pSegArr->numSegments].end = i;
		pSegArr->segments[pSegArr->numSegments].pObject = NULL;
		if (i == widthGrey)
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
struct object * findObjects(uint8 const value) {
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
	
	segsCurr->numSegments = 0;
	
	for (i = 0; i < heightGrey; i += 1) /* this loops over every line, starting from the second */
	{ /* both segsLast and segsCurr point to a valid aSegment instance */
		struct object * obj = NULL; /* This holds the object for the last segment processed. */
		
		/* swap the pointers to the last and the current segment array */
		s_segmentArray * segmentsTemp = segsLast;
		segsLast = segsCurr;
		segsCurr = segmentsTemp;
		findSegments(data.imgGrey + i * widthGrey, value, segsCurr);
		
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

void classifyObjects(uint8 const * const pImgRaw, struct object * const pObj, uint32 const thresholdWeight, t_index const spotSize)
{
	struct object * obj;
	
	for (obj = pObj; obj != NULL; obj = obj->pNext)
	{
		obj->posWghtX /= obj->weight;
		obj->posWghtY /= obj->weight;
		
		if (obj->weight < thresholdWeight)
			obj->classification = e_classification_tooSmall;
		else
		{
			int16 posX, posY;
			uint8 color[3], mid;
			t_index maxComp, minComp, midComp;
			
			posX = 2 * obj->posWghtX - spotSize / 2;
			posY = 2 * obj->posWghtY - spotSize / 2;
			
			/* Move the spot inside the picture. */
			if (posX < 0)
				posX = 0;
			if (posY < 0)
				posY = 0;
			if (posX + spotSize >= WIDTH_CAPTURE)
				posX = WIDTH_CAPTURE - spotSize;
			if (posY + spotSize >= HEIGHT_CAPTURE)
				posY = HEIGHT_CAPTURE - spotSize;
			
			OscVisDebayerSpot(pImgRaw, WIDTH_CAPTURE, HEIGHT_CAPTURE, data.enBayerOrder, posX, posY, spotSize, color);
			
			obj->color.red = color[2];
			obj->color.green = color[1];
			obj->color.blue = color[0];
			
			/* Find out which color components are the largest and the smallest. */
			if (color[0] > color[1])
				if (color[0] > color[2])
				{
					maxComp = 0;
					if (color[1] > color[2])
					{
						minComp = 2;
						midComp = 1;
					}
					else
					{
						minComp = 1;
						midComp = 2;
					}
				}
				else
				{
					maxComp = 2;
					minComp = 1;
					midComp = 0;
				}
			else
				if (color[1] > color[2])
				{
					maxComp = 1;
					if (color[0] > color[2])
					{
						minComp = 2;
						midComp = 0;
					}
					else
					{
						minComp = 0;
						midComp = 2;
					}
				}
				else
				{
					maxComp = 2;
					minComp = 0;
					midComp = 1;
				}
			
			mid = (uint16) (color[midComp] - color[minComp]) * 255 / (color[maxComp] - color[minComp]);
			
			/* Classify the object according to which color components are the largest and smallest and where in between to other lies */
			if (minComp == 0)
				if (maxComp == 2)
					if (mid < 72)
						obj->classification = e_classification_sugusRed;
					else if (mid < 156)
						obj->classification = e_classification_sugusOrange;
					else
						obj->classification = e_classification_sugusYellow;
				else if (maxComp == 1)
					if (mid < 166)
						obj->classification = e_classification_sugusGreen;
					else
						obj->classification = e_classification_sugusYellow;
				else
					obj->classification = e_classification_otherColor;
			else
				obj->classification = e_classification_otherColor;
		}
	}
}

void drawRectangle(uint8 * const pImg, t_index const width, t_index const left, t_index const right, t_index const top, t_index const bottom, s_color const color)
{
	uint16 i;
	
	/* Draw the horizontal lines. */
	for (i = left; i < right; i += 1)
	{
		pImg[(width * top + i) * 3] = color.blue;
		pImg[(width * top + i) * 3 + 1] = color.green;
		pImg[(width * top + i) * 3 + 2] = color.red;
		
		pImg[(width * (bottom - 1) + i) * 3] = color.blue;
		pImg[(width * (bottom - 1) + i) * 3 + 1] = color.green;
		pImg[(width * (bottom - 1) + i) * 3 + 2] = color.red;
	}
	
	/* Draw the vertical lines. */
	for (i = top; i < bottom; i += 1)
	{
		pImg[(width * i + left) * 3] = color.blue;
		pImg[(width * i + left) * 3 + 1] = color.green;
		pImg[(width * i + left) * 3 + 2] = color.red;
		
		pImg[(width * i + right - 1) * 3] = color.blue;
		pImg[(width * i + right - 1) * 3 + 1] = color.green;
		pImg[(width * i + right - 1) * 3 + 2] = color.red;
	}
}

void fillRectangle(uint8 * const pImg, t_index const width, t_index const left, t_index const right, t_index const top, t_index const bottom, s_color const color)
{
	uint16 ix, iy;
	
	/* Draw the vertical lines. */
	for (iy = top; iy < bottom; iy += 1)
	{
		for (ix = left; ix < right; ix += 1)
		{
			pImg[(width * iy + ix) * 3] = color.blue;
			pImg[(width * iy + ix) * 3 + 1] = color.green;
			pImg[(width * iy + ix) * 3 + 2] = color.red;
		}
	}
}

void writeNiceDebugPicture(uint8 const * const pRawImg, struct object * const pObjs, t_index const spotSize)
{
	/* Maximize the saturation of a color. Black remains black. */
	s_color saturate(s_color const color)
	{
		uint8 const maxComp = max(color.red, max(color.green, color.blue));
		uint8 const minComp = min(color.red, min(color.green, color.blue));
		s_color colRet;
		
		colRet.red = (uint16) (color.red - minComp) * 255 / (maxComp - minComp);
		colRet.green = (uint16) (color.green - minComp) * 255 / (maxComp - minComp);
		colRet.blue = (uint16) (color.blue - minComp) * 255 / (maxComp - minComp);
		
		return colRet;
	}

	/*! @brief A buffer to hold the resulting color image. */
	struct object * obj;
	t_index ix, iy;
	bool const markAreas = FALSE;
	
	/* Use the framework function to debayer the image. */
	OscVisDebayer(pRawImg, WIDTH_CAPTURE, HEIGHT_CAPTURE, data.enBayerOrder, data.imgColor);
	
	/* Mark areas that are considered part of an object. */
	if (markAreas)
		for (iy = 0; iy < heightGrey; iy += 1)
			for (ix = 0; ix < widthGrey; ix += 1)
			{
				uint8 const grey = data.imgGrey[iy * widthGrey + ix];
				uint32 const pos = (iy * WIDTH_CAPTURE + ix) * 6;;
				
				if (grey == 0)
				{
					data.imgColor[pos + 0] = 0;
					data.imgColor[pos + 1] = 0;
					data.imgColor[pos + 2] = ~0;
				}
			}
	
	for (obj = pObjs; obj != NULL; obj = obj->pNext)
	{
		s_color const green = { 0, ~0, 0 }, red = { 0, 0, ~0 }, black = { 0, 0, 0 },
			white = { ~0, ~0, ~0 };
		
		/* Objects that are too near to the border. */
		if (obj->classification == e_classification_tooNearToBorder)
			continue;
		
		/* Objects that are too small. */
		if (obj->classification == e_classification_tooSmall)
		{
			if (obj->weight > 100)
				drawRectangle(data.imgColor, WIDTH_CAPTURE, obj->left * 2, obj->right * 2, obj->top * 2, obj->bottom * 2, red);
			
			continue;
		}
		
		{
			s_color const color = obj->color;
			s_color colorFill, colorBorder;
			int16 spotPosX, spotPosY;
			
			drawRectangle(data.imgColor, WIDTH_CAPTURE, obj->left * 2, obj->right * 2, obj->top * 2, obj->bottom * 2, green);
			
		//	colorFill = saturate(color);
			colorFill = color;
			colorBorder = (color.red + color.green + color.blue) > (255 * 3 / 2) ? black : white;
			
			spotPosX = obj->posWghtX * 2 - spotSize / 2;
			spotPosY = obj->posWghtY * 2 - spotSize / 2;
			
			/* Move the rectangle inside the picture. */
			if (spotPosX < 0)
				spotPosX = 0;
			if (spotPosY < 0)
				spotPosY = 0;
			if (spotPosX + spotSize >= WIDTH_CAPTURE)
				spotPosX = WIDTH_CAPTURE - spotSize;
			if (spotPosY + spotSize >= HEIGHT_CAPTURE)
				spotPosY = HEIGHT_CAPTURE - spotSize;
			
			/* draws a rectangle filled with the color found */
			fillRectangle(data.imgColor, WIDTH_CAPTURE, spotPosX, spotPosX + spotSize, spotPosY, spotPosY + spotSize, colorFill);
			drawRectangle(data.imgColor, WIDTH_CAPTURE, spotPosX, spotPosX + spotSize, spotPosY, spotPosY + spotSize, colorBorder);
		}
	}
	
	{
		struct OSC_PICTURE pic;
		
		pic.width = WIDTH_CAPTURE;
		pic.height = HEIGHT_CAPTURE;
		pic.type = OSC_PICTURE_RGB_24;
		pic.data = (void *) data.imgColor;
		
		OscBmpWrite(&pic, IMG_FILENAME "~");
		rename(IMG_FILENAME "~", IMG_FILENAME);
	}
}

void writeNiceDebugPictureHalfSize(uint8 const * const pRawImg, t_index const width, t_index const height, uint8 * const pOut) {
	
}

void process(uint8 const * const pRawImg)
{
	OSC_ERR err;
	
	uint8 const thresholdValue = 50;
	uint32 const thresholdWeight = 500;
	
	err = OscCamGetBayerOrder(&data.enBayerOrder, 0, 0);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Error getting bayer order! (%d)\n", __func__, err);
		return;
	}
	
	err = OscVisDebayerGreyscaleHalfSize(pRawImg, WIDTH_CAPTURE, HEIGHT_CAPTURE, data.enBayerOrder, data.imgGrey);
	
	/* masks parts of the image that contain an objcet */
	applyThreshold(thresholdValue, FALSE, TRUE);
	
	{
		struct object * objs = findObjects(~0), * obj;
		uint16 valves = 0;
		
		classifyObjects(pRawImg, objs, thresholdWeight, 8);
		writeNiceDebugPicture(pRawImg, objs, 8);
		
		/* Print a line for each found object. */
		for (obj = objs; obj != NULL; obj = obj->pNext)
			if (obj->classification != e_classification_tooSmall)
			{
			//	printf("Left: %u, Right: %u, Top: %u, Bottom: %u, Weight: %lu, Color: (%u, %u, %u)\n", obj->left, obj->right, obj->top, obj->bottom, obj->weight, obj->color.red, obj->color.green, obj->color.blue);
				printf("Weight: %lu, Color: (%u, %u, %u) ", obj->weight, obj->color.red, obj->color.green, obj->color.blue);
				
				if (obj->classification == e_classification_sugusRed)
					printf("-> red\n");
				else if (obj->classification == e_classification_sugusOrange)
					printf("-> orange\n");
				else if (obj->classification == e_classification_sugusYellow)
					printf("-> yellow\n");
				else if (obj->classification == e_classification_sugusGreen)
					printf("-> green\n");
			}
		
		for (obj = objs; obj != NULL; obj = obj->pNext)
			if (obj->classification != e_classification_tooSmall)
				valves = valves << 1 | 0x0001;
		
		modbus_sendMessage(valves);
		
		printf("\n");
	}
}

void process_init() {
//	segmentArrays_init ();
}
