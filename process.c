/*! @file process.c
 * @brief Contains the actual algorithm and calculations.
 */

#define ASSERTS_ENABLE

#include "valves.h"
#include "config.h"
#include "process.h"

/*! @brief The file name the debug image should be written to. */
#define IMG_FILENAME "/home/httpd/image.bmp"
/*! @brief The width of the debayered grayscale image. */
#define WIDTH_GREY (WIDTH_CAPTURE / 2)
/*! @brief The height of the debayered grayscale image. */
#define HEIGHT_GREY (HEIGHT_CAPTURE / 2)

struct {
	/*! @brief Bayer order of the captured image. */
	enum EnBayerOrder enBayerOrder;
	/*! @brief Buffer for the debayered color image. */
	uint8 imgColor[3 * WIDTH_CAPTURE * HEIGHT_CAPTURE];
	/*! @brief Buffer for the greyscale image with half the with an height */
	uint8 imgGrey[WIDTH_GREY * HEIGHT_GREY];
} data;

/*! @brief This classifies an object. */
typedef enum {
	e_classification_sugusRed,
	e_classification_sugusGreen,
	e_classification_sugusOrange,
	e_classification_sugusYellow,
	e_classification_unknown,
	e_classification_tooSmall,
} e_classification;

/*! @brief This contains a color value. */
typedef struct {
	uint8 blue, green, red;
} s_color;

/*! @brief This holds the segments found in one line. */
typedef struct {
	/*!  @brief A segment specifies one part of a line that contains an object. */
	struct segment {
		/*! @brief These contain the first pixels that are and aren't part of teh object respectively. */
		t_index begin, end;
		/*! @brief This contains a reference to the object the segment has been assigned to. */
		struct object * pObject;
	} segments[100];
	
	/*! @brief Number of segments the array contains. */
	t_index numSegments;
} s_segmentArray;

/*! @brief One instance of s_objectPool holds multiple linked lists of objects which may be moved between the lists. */
typedef struct {
	/*! @brief One object recognized by the algorithm. */
	struct object {
		/*! @brief Boundaries of the object, ie. all points recognized as beeing parts of the object lie within these boundaries. */
		uint16 left, right, top, bottom;
		/*! @brief Sum of the positions of all points part of this object. */
		uint32 posWghtX, posWghtY;
		/*! @brief Number of points part of this object. */
		uint32 weight;
		/*! @brief Color of the object as measured in the center of the object. */
		s_color color;
		/*! @brief How the object has been classified. */
		e_classification classification;
		/*! @brief Whether the object has been found to be a duplicate of an object seen in a previous image. This prevents an object from beeing counted multiple times. */
		bool isDuplicate;
		/*! @brief Pointers to adjacent elements in the linked list. */
		struct object * pPrev, * pNext;
	} objects[100];
	
	/*! @brief Pointers the first objects in the linked lists. */
	struct object * pFirst[3];
} s_objectPool;

s_objectPool objPool;

/*! @brief Dumps the object pool pPool for debugging purposes. */
void objectPool_dump(s_objectPool const * const pPool)
{
	t_index i;
	
	/* Dumps all the pointers to the heads of the linked lists. */
	for (i = 0; i < length (pPool->pFirst); i += 1)
		printf("pFirst[%d]: %ld\n", i, pPool->pFirst[i] - (pPool->objects));
	
	/* Dumps all objects and their relationships. */
	for (i = 0; i < length(pPool->objects); i += 1)
	{
		printf("%u: %ld, %ld\n", i, pPool->objects[i].pPrev - (pPool->objects), pPool->objects[i].pNext - (pPool->objects));
	}
}

/*! @brief Initializes the object pool pPool to a state where all objects are in the first list. */
void objectPool_init (s_objectPool * const pPool)
{
	t_index i;
	
	/* Link all the objects in consecutive order. */
	for (i = 1; i < length (pPool->objects); i += 1)
	{
		pPool->objects[i - 1].pNext = pPool->objects + i;
		pPool->objects[i].pPrev = pPool->objects + i - 1;
	}
	
	/* Sets all the pointers to the first elements in the lists but the first to NULL. */
	pPool->pFirst[0] = pPool->objects;
	for (i = 1; i < length (pPool->pFirst); i += 1)
		pPool->pFirst[i] = NULL;
	
	/* Sets the ends of the list to point to NULL. */
	pPool->objects[0].pPrev = NULL;
	pPool->objects[length(pPool->objects) - 1].pNext = NULL;
}

/*!
 * @brief Moves an object from one list to another.
 * 
 * @param pPool Pointer to the object pool the object belongs to.
 * @param pObj Pointer to the object to move.
 * @param from List the object is currently in.
 * @param to List the object should be moved to.
 */
void objectPool_move (s_objectPool * const pPool, struct object * const pObj, t_index from, t_index to)
{
	/* First, we remove the object from the from list. */
	if (pObj->pPrev == NULL)
		/* The object is at the begining of the list. */
		pPool->pFirst[from] = pObj->pNext;
	else
		/* The object is somewhere else in the list. */
		pObj->pPrev->pNext = pObj->pNext;
	
	if (pObj->pNext != NULL)
		/* The object is not at the end of the list. */
		pObj->pNext->pPrev = pObj->pPrev;
	
	/* And then we put it into the active list. */
	pObj->pPrev = NULL;
	pObj->pNext = pPool->pFirst[to];
	if (pObj->pNext != NULL)
		pObj->pNext->pPrev = pObj;
	pPool->pFirst[to] = pObj;
}

/*!
 * @brief Find segments in one line of a grayscale picture.
 *
 * @param pImg Pointer to the first pixel of the line to find the segments in.
 * @param value The minimum value to be considered part of a segment.
 * @param pSegArr A pointer to the segment array the segments will be returned in.
 */
void findSegments(uint8 const * const pImg, uint8 const value, s_segmentArray * const pSegArr)
{
	uint16 i = 0;
	
	/* This loops the maximum number of segments in the segment array. */
	for (pSegArr->numSegments = 0; pSegArr->numSegments < length (pSegArr->segments); pSegArr->numSegments += 1)
	{
		/* This loops to the first point to be part of a segment. */
		while (i < WIDTH_GREY && pImg[i] < value)
			i += 1;
		pSegArr->segments[pSegArr->numSegments].begin = i;
		if (i == WIDTH_GREY)
			break;
		
		/* This loops to the first point not part of a segment anymore. */
		while (i < WIDTH_GREY && pImg[i] >= value)
			i += 1;
		/* We ended a segment, possibly at the end of the line */
		pSegArr->segments[pSegArr->numSegments].end = i;
		pSegArr->segments[pSegArr->numSegments].pObject = NULL;
		if (i == WIDTH_GREY)
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
 * This function finds object in the grayscale image data.imgGrey where an object consists on pixels that are at least as bright as value.
 *
 * @param pPool Object pool to get the objects from.
 * @param value Threshold to decide what is part of an object.
 * @return A pointer to the first object in the linked list of found objects.
 */
struct object * findObjects(s_objectPool * const pPool, uint8 const value) {
	/*! @brief Creates an object from a single segment. */
	inline struct object * createObjectForSegment(t_index line, struct segment * pSeg, s_objectPool * pObjPool)
	{
		struct object * obj = pObjPool->pFirst[0];
		
		/* Check whether there are object left in the list of inactive objects. */
		if (obj != NULL)
		{
			/* Set the object's parameters according to the segment. */
			obj->left = pSeg->begin;
			obj->right = pSeg->end;
			obj->weight = obj->right - obj->left;
			obj->top = line;
			obj->bottom = line + 1;
			obj->posWghtX = obj->weight * (obj->left + obj->right) / 2;
			obj->posWghtY = obj->weight * line;
			obj->isDuplicate = false;
			
			/* Move the segment to the list of active objects and assign the segment to the object. */
			objectPool_move(pObjPool, obj, 0, 1);
			pSeg->pObject = obj;
		}
		
		return obj;
	}
	
	/*! @brief Merges two objects and re-labels the segments given. */
	inline struct object * mergeObjects(struct object * pObj1, struct object * pObj2, s_objectPool * pObjPool, s_segmentArray * pSegArr1, s_segmentArray * pSegArr2)
	{
		/* Check whether we actualy have only one object in which case we just return that one. */
		if (pObj1 == NULL)
			return pObj2;
		else if (pObj2 == NULL)
			return pObj1;
		else if (pObj1 != pObj2)
		{	/* If the objects are not the same the second is merged into the first one. */
			t_index i;
			
			/* Here we merge the objects' parameters. */
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
			
			/* Move the merged object in to the inactive list. */
			objectPool_move (pObjPool, pObj2, 1, 0);
		}
		
		return pObj1;
	}
	
	t_index i;
	t_index iLast, iCurrent;
	
	/* These two segment arrays contain the segments of the last an the current line. */
	static s_segmentArray segArrs[2];
	s_segmentArray * segsLast = segArrs, * segsCurr = segArrs + 1;
	
	/* Here we empty the list of objects of two images before. */
	while (pPool->pFirst[2] != NULL)
		objectPool_move (pPool, pPool->pFirst[2], 2, 0);
	
	/* And the objects of the last image to the list for the images of the last object. */
	pPool->pFirst[2] = pPool->pFirst[1];
	pPool->pFirst[1] = NULL;
	
	segsCurr->numSegments = 0;
	
	/* this loops over every line, starting from the second */
	for (i = 0; i < HEIGHT_GREY; i += 1)
	{	/* both segsLast and segsCurr point to a valid aSegment instance */
		struct object * obj = NULL; /* This holds the object for the last segment processed. */
		
		/* swap the pointers to the last and the current segment array */
		s_segmentArray * segmentsTemp = segsLast;
		segsLast = segsCurr;
		segsCurr = segmentsTemp;
		findSegments(data.imgGrey + i * WIDTH_GREY, value, segsCurr);
		
		iLast = iCurrent = 0;
		
		/* this loops over all segments of the last and current line */
		while (iLast < segsLast->numSegments || iCurrent < segsCurr->numSegments)
		{	/* both segsLast and segsCurr point to a valid aSegment instance, but iLast and iCurrent may point past the end of the array. */
			/* First we check, whether we moved on the current or on the last line the last step. */
			
			if (iCurrent < segsCurr->numSegments)
			{	/* There are segments on the current line. */
				if (obj == NULL)
				{	/* This means that we either moved on the current line or that we are on the first segment. So we create an object for the lower line. */
					obj = createObjectForSegment(i, segsCurr->segments + iCurrent, &objPool);
				}
				
				/* So we check whether we also have segments on the last line. */
				if (iLast < segsLast->numSegments)
				{	/* We do have segments on the last and the current line so we check if they overlap. */
					if (segsLast->segments[iLast].begin
							< segsCurr->segments[iCurrent].end
						&& segsCurr->segments[iCurrent].begin
							< segsLast->segments[iLast].end)
					{	/* They do overlap so we merge the segment from the current line into the object from the segment from the last. */
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
					{	/* They do not overlap so we just have to create a new object for the segment of the current line. */
						/* We need to check which segment ends first. */
						if (segsLast->segments[iLast].end
							< segsCurr->segments[iCurrent].end)
						{	/* The segment on the last line ends first, so we just skip the segment from the last line. */
							iLast += 1;
						}
						else
						{	/* The segment on the current line ends first. So we will have to generate a new object for the next segment on the current line. */
							obj = NULL;
							iCurrent += 1;
						}
					}
				}
				else
				{	/* We only have segments on the current line. This menas that we have to generate a new object for the next segment. */
					obj = NULL;
					iCurrent += 1;
				}
			}
			else
			{	/* There are no segments on the current line left so we just skip the ones from the last line. */
				iLast += 1;
			}
		}
	}
	
	return objPool.pFirst[1];
}

/*!
 * @brief Classify objects according to their size and color.
 *
 * @param pImgRaw Pointer to the image captured from the camera. Used to measure the color of images.
 * @param pObj Pointer to the first object of the list of objects to be classified.
 * @param thresholdWeight Minimum weight of an object to be considered.
 * @param spotSize Size of the spot to debayer for measuring the color.
 */
void classifyObjects(uint8 const * const pImgRaw, struct object * const pObj, uint32 const thresholdWeight, t_index const spotSize)
{
	inline int32 dotProd(uint8 const * const vec1, int32 const * const vec2)
	{
		return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
	}
	
	struct object * obj;
	
	for (obj = pObj; obj != NULL; obj = obj->pNext)
	{
		obj->posWghtX /= obj->weight;
		obj->posWghtY /= obj->weight;
		
		if (obj->weight < thresholdWeight)
		{
			obj->classification = e_classification_tooSmall;
		}
		else
		{
			uint8 color[3];
			int16 posX, posY;
			int32 planes[3][3] = {
				{ -3288, 6429, -4160 }, /* Between green and yellow and orange and red. */
				{ -141, 7330, -7662 }, /* Between green and yellow. */
				{ -782105, 575153, -64151 } /* Between orange and red. */
			};
			
			posX = 2 * obj->posWghtX - spotSize / 2;
			posY = 2 * obj->posWghtY - spotSize / 2;
			
			/* Move the spot inside the picture. */
			if (posX < 0)
				posX = 0;
			else if (posX + spotSize >= WIDTH_CAPTURE)
				posX = WIDTH_CAPTURE - spotSize;
			if (posY < 0)
				posY = 0;
			else if (posY + spotSize >= HEIGHT_CAPTURE)
				posY = HEIGHT_CAPTURE - spotSize;
			
			OscVisDebayerSpot(pImgRaw, WIDTH_CAPTURE, HEIGHT_CAPTURE, data.enBayerOrder, posX, posY, spotSize, color);
			
			obj->color.red = color[2];
			obj->color.green = color[1];
			obj->color.blue = color[0];
			
			if (dotProd(color, planes[0]) > 255)
				if (dotProd(color, planes[1]) > 255)
					obj->classification = e_classification_sugusGreen;
				else
					obj->classification = e_classification_sugusYellow;
			else
				if (dotProd(color, planes[2]) > 255)
					obj->classification = e_classification_sugusOrange;
				else
					obj->classification = e_classification_sugusRed;
			
		//	obj->classification = e_classification_unknown;
		}
	}
}

/*!
 * @brief Draws a rectangle into a color image.
 *
 * This function draws the border of a rectangle with the specified color into an image. The rectanlge covers the area { (x, y) | left <= x < right && top <= y < bottom }
 *
 * @param pImg Pointer to the image to draw the rectangle in.
 * @param width Width of the image.
 * @param left Left coordinate of the rectanle.
 * @param right Right coordinate of the rectanle.
 * @param top Top coordinate of the rectanle.
 * @param bottom Bottom coordinate of the rectanle.
 * @param color Color to draw the rectangle with.
 */
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

/*!
 * @brief Fills a rectangle into a color image.
 *
 * This function fills a rectangle with the specified color in an image. The rectanlge covers the area { (x, y) | left <= x < right && top <= y < bottom }
 *
 * @param pImg Pointer to the image to draw the rectangle in.
 * @param width Width of the image.
 * @param left Left coordinate of the rectanle.
 * @param right Right coordinate of the rectanle.
 * @param top Top coordinate of the rectanle.
 * @param bottom Bottom coordinate of the rectanle.
 * @param color Color to draw the rectangle with.
 */
void fillRectangle(uint8 * const pImg, t_index const width, t_index const left, t_index const right, t_index const top, t_index const bottom, s_color const color)
{
	uint16 ix, iy;
	
	/* Fill each pixel. */
	for (iy = top; iy < bottom; iy += 1)
		for (ix = left; ix < right; ix += 1)
		{
			pImg[(width * iy + ix) * 3] = color.blue;
			pImg[(width * iy + ix) * 3 + 1] = color.green;
			pImg[(width * iy + ix) * 3 + 2] = color.red;
		}
}

/*!
 * @brief This writes an image for debugging purposes.
 *
 * Found object will be surrounded with a green border and the color measuremenet point will be marked by a rectangle filled with the color the object has been classified into.
 *
 * @param pRawImg Pointer to the image to draw the rectangle in.
 * @param pObjs Objects to draw into the debug picture.
 * @param spotSize Size the color spots should be drawn with.
 */
void writeNiceDebugPicture(uint8 const * const pRawImg, struct object * const pObjs, t_index const spotSize)
{
	/*! @brief Maximize the saturation of a color. Black remains black. */
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
	
	struct object * obj;
	t_index ix, iy;
	bool const markAreas = FALSE;
	
	/* Use the framework function to debayer the image. */
	OscVisDebayer(pRawImg, WIDTH_CAPTURE, HEIGHT_CAPTURE, data.enBayerOrder, data.imgColor);
	
	/* Mark areas that are considered part of an object. */
	if (markAreas)
		for (iy = 0; iy < HEIGHT_GREY; iy += 1)
			for (ix = 0; ix < WIDTH_GREY; ix += 1)
			{
				uint8 const grey = data.imgGrey[iy * WIDTH_GREY + ix];
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
		s_color const green = { 0, ~0, 0 }, yellow = { 0, ~0, ~0 }, orange = { 0, ~0 / 2, ~0 }, red = { 0, 0, ~0 }, black = { 0, 0, 0 },
			white = { ~0, ~0, ~0 };
		
		/* Objects that are too small. */
		if (obj->classification == e_classification_tooSmall)
		{
			if (obj->weight > 100)
				drawRectangle(data.imgColor, WIDTH_CAPTURE, obj->left * 2, obj->right * 2, obj->top * 2, obj->bottom * 2, red);
		}
		else
		{
		//	s_color const color = obj->color;
			s_color colorFill, colorBorder;
			int16 spotPosX, spotPosY;
			s_color color;
			
			if (obj->classification == e_classification_sugusGreen)
				color = green;
			else if (obj->classification == e_classification_sugusYellow)
				color = yellow;
			else if (obj->classification == e_classification_sugusOrange)
				color = orange;
			else if (obj->classification == e_classification_sugusRed)
				color = red;
			else
				color = black;
			
			drawRectangle(data.imgColor, WIDTH_CAPTURE, obj->left * 2, obj->right * 2, obj->top * 2, obj->bottom * 2, green);
			
			colorFill = color;
			colorBorder = (color.red + color.green + color.blue) > (255 * 3 / 2) ? black : white;
			
			spotPosX = obj->posWghtX * 2 - spotSize / 2;
			spotPosY = obj->posWghtY * 2 - spotSize / 2;
			
			/* Move the rectangle inside the picture. */
			if (spotPosX < 0)
				spotPosX = 0;
			else if (spotPosX + spotSize >= WIDTH_CAPTURE)
				spotPosX = WIDTH_CAPTURE - spotSize;
			if (spotPosY < 0)
				spotPosY = 0;
			else if (spotPosY + spotSize >= HEIGHT_CAPTURE)
				spotPosY = HEIGHT_CAPTURE - spotSize;
			
			/* draws a rectangle filled with the color found */
			fillRectangle(data.imgColor, WIDTH_CAPTURE, spotPosX, spotPosX + spotSize, spotPosY, spotPosY + spotSize, colorFill);
			drawRectangle(data.imgColor, WIDTH_CAPTURE, spotPosX, spotPosX + spotSize, spotPosY, spotPosY + spotSize, colorBorder);
		}
	}
	
	/* Write the image. */
	{
		struct OSC_PICTURE pic;
		
		pic.width = WIDTH_CAPTURE;
		pic.height = HEIGHT_CAPTURE;
		pic.type = OSC_PICTURE_RGB_24;
		pic.data = (void *) data.imgColor;
		
		/* We first write the image to a journal file and the move it over to the actual path atomically. */
		OscBmpWrite(&pic, IMG_FILENAME "~");
		rename(IMG_FILENAME "~", IMG_FILENAME);
	}
}

/*!
 * @brief Calculates when an object will be in fornt of the valves and lets the valves be activated at that time.
 *
 * @param pObj The object to insert into the valve queue.
 * @param capture_time Time the object has been seen.
 */
inline void insertIntoValves(struct object * pObj, t_time capture_time)
{
	/* gives the time needed from the conveyor belt to the position. */
	inline t_time posToTime(int16 const pos)
	{
		return (TIME_TO_BOTTOM_OF_PICTURE - TIME_TO_TOP_OF_PICTURE) / HEIGHT_GREY * pos + TIME_TO_TOP_OF_PICTURE;
	}
	
	/* Gives the valve responsible for something at the position. */
	inline t_index posToValve(int16 const pos)
	{
		return (pos - PIXEL_BEGIN_FIRST_VALVE) * 16 / (PIXEL_END_LAST_VALVE - PIXEL_BEGIN_FIRST_VALVE);
	}
	
	t_time const time_top = TIME_TO_VALVES - posToTime(pObj->top);
	t_time const time_bottom = TIME_TO_VALVES - posToTime(pObj->bottom);
	t_index const valve_begin = max(0, posToValve(pObj->left) - 1);
	t_index const valve_end = min(15, posToValve(pObj->right) + 1);
	
	if (time_bottom > time_top)
		printf("Top: %d, Bottom: %d\n", pObj->top, pObj->bottom);
	
	valves_insertEvent(capture_time + time_bottom, capture_time + time_top, valve_begin, valve_end);
}

/*!
 * @brief Marks objects in the list from when they are also in the list with.
 *
 * This function looks for objects that were seen in multiple frames.
 *
 * @param from Objects in this list will be marked.
 * @param with Objects from the previous image.
 * @param timeDelta Time difference the images have been taken in.
 * @param maxDistSqr Maximum distance the object may have diverged to be recognized as the same, squared.
 */
inline void removeDuplicates(struct object * const from, struct object const * const with, t_time const timeDelta, uint32 const maxDistSqr)
{
	struct object * obj1;
	struct object const * obj2;
	t_index const posDelta = (timeDelta >> 8) * HEIGHT_GREY / ((TIME_TO_BOTTOM_OF_PICTURE - TIME_TO_TOP_OF_PICTURE) >> 8); /* We need to gain some space so the numbers don't get too big, there is probably a better way to do this... */
	
	/* This loops over each combination of an object from this frame and and object from the last frame. */
	for (obj1 = from; obj1 != NULL; obj1 = obj1->pNext)
		for (obj2 = with; obj2 != NULL; obj2 = obj2->pNext)
		{
			t_index const xDist = obj2->posWghtX - obj1->posWghtX;
			t_index const yDist = obj2->posWghtY - obj1->posWghtY + posDelta;
			uint32 dist = (uint32) xDist * xDist + yDist * yDist;
			
			/* The object from this frane is only marked as a duplicate if both of them would count as an object. */
			if (dist < maxDistSqr)
				if (obj1->classification != e_classification_tooSmall && obj2->classification != e_classification_tooSmall)
					obj1->isDuplicate = true;
		}
}

/*!
 * @brief This is the main function that processes the captured images.
 *
 * @param pRawImg Pointer to the raw image captured by the sensor.
 * @param capture_time Time as which the image has been taken.
 */
void process(uint8 const * const pRawImg, t_time capture_time)
{
	OSC_ERR err;
	
	uint8 const thresholdValue = 40;
	uint32 const thresholdWeight = 500;
	
benchmark_init;
	
	/* Debayers the image to a greyscale image with half the dimensions. */
	err = OscVisDebayerGreyscaleHalfSize(pRawImg, WIDTH_CAPTURE, HEIGHT_CAPTURE, data.enBayerOrder, data.imgGrey);

benchmark_delta;
	
	valves_handleValves();

	{
		/* Detect the objects on the image. */
		struct object * const objs = findObjects(&objPool, thresholdValue), * obj;
		static t_time last_capture_time = 0;
		
		/* Classify the objects according to their size and color. */
		classifyObjects(pRawImg, objs, thresholdWeight, 8);
		
		/* Remove duplicate objects */
		removeDuplicates(objPool.pFirst[1], objPool.pFirst[2], capture_time - last_capture_time, 45 * 45);
		last_capture_time = capture_time;
	
	benchmark_delta;
		
		/* Print a line for each found object. */
		for (obj = objs; obj != NULL; obj = obj->pNext)
		{
			/* Print a line for each object. */
			printf("Position: (%lu, %lu), Weight: %lu, Color: (%u, %u, %u)", obj->posWghtX, obj->posWghtY, obj->weight, obj->color.red, obj->color.green, obj->color.blue);
			
			if (obj->classification == e_classification_tooSmall)
				printf(" -> too small");
			else if (obj->classification == e_classification_sugusGreen)
				printf(" -> green");
			else if (obj->classification == e_classification_sugusYellow)
				printf(" -> yellow");
			else if (obj->classification == e_classification_sugusOrange)
				printf(" -> orange");
			else if (obj->classification == e_classification_sugusRed)
				printf(" -> red");
			else if (obj->classification == e_classification_unknown)
				printf(" -> unknown");
			
			if (obj->isDuplicate)
				printf(" (duplicate)\n");
			else
				printf("\n");
			
			/* Update the counters. */
			if (! obj->isDuplicate)
			{
				if (obj->classification == e_classification_sugusGreen)
					configuration.count_color[0] += 1;
				else if (obj->classification == e_classification_sugusYellow)
					configuration.count_color[1] += 1;
				else if (obj->classification == e_classification_sugusOrange)
					configuration.count_color[2] += 1;
				else if (obj->classification == e_classification_sugusRed)
					configuration.count_color[3] += 1;
				else if (obj->classification == e_classification_unknown)
					configuration.count_unknown += 1;
			}
			
			/* Check whether we should sort out this object. */
			if ((obj->classification == e_classification_sugusGreen) && configuration.sort_color[0] || (obj->classification == e_classification_sugusYellow) && configuration.sort_color[1] || (obj->classification == e_classification_sugusOrange) && configuration.sort_color[2] || (obj->classification == e_classification_sugusRed) && configuration.sort_color[3] || (obj->classification == e_classification_unknown) && configuration.sort_unknown)
			{
				insertIntoValves(obj, capture_time);
				
				/* Update the counter of sorted objects.. */
				if (! obj->isDuplicate)
					configuration.count_sorted += 1;
			}
		}
		
		/* Write out an image to be picked up by the web interface if we're in calibration mode. */
		if (configuration.calibrating)
			writeNiceDebugPicture(pRawImg, objs, 8);
			
	benchmark_delta;
	
		printf("\n");
	}
}

/*! @brief Initialization function for the processing subsystem. */
void process_init() {
	OSC_ERR err = OscCamGetBayerOrder(&data.enBayerOrder, 0, 0);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "%s: Error getting bayer order! (%d)\n", __func__, err);
		return;
	}
	
	objectPool_init(&objPool);
}
