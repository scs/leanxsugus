/*! @file process_frame.c
 * @brief Contains the actual algorithm and calculations.
 */

#include "process_frame.h"
// #include "framework_extensions.h"

#define POOL_SEGMENTS_COUNT 100

struct segment {
	uint16 begin, end;
	struct segment * pNext;
};

static struct {
	struct segment * pFirstEmpty;
	struct segment aSegments[POOL_SEGMENTS_COUNT];
} segments_pool;

void segments_pool_init ()
{
	int i;
	
	for (i = 1; i < POOL_SEGMENTS_COUNT; i += 1)
	{
		segments_pool.aSegments[i - 1].pNext = segments_pool.aSegments + i;
	}
	
	segments_pool.pFirstEmpty = segments_pool.aSegments;
	segments_pool.aSegments[POOL_SEGMENTS_COUNT - 1].pNext = NULL;

/*	printf ("0x%x\n", segments_pool.pFirstEmpty);
	printf ("0x%x\n", segments_pool.aSegments[0].pNext);
	printf ("0x%x\n", segments_pool.aSegments[99].pNext);
*/
}

struct segment * segments_pool_get ()
{
	struct segment * seg = segments_pool.pFirstEmpty;
	
	if (seg != NULL)
	{
		segments_pool.pFirstEmpty = seg->pNext;
		seg->pNext = NULL;
	}
	
	return seg;
}

void segments_pool_put (struct segment * const seg)
{
	seg->pNext = segments_pool.pFirstEmpty;
	segments_pool.pFirstEmpty = seg;
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
 * @return A pointer to the first segment found as a linked list.
 */
struct segment * findSegments (uint8 const * const pImg, uint16 const width, uint8 const value)
{
	uint16 i;
	struct segment * pSegCurrent, * pSegFirst;
	
	for (i = 0; i < width; i += 1)
	{
		lbl_outSegment:
		
		if (pImg[i] == value)
		{
			// we have to check for NULL returned by segments_pool_get()
			if (pSegFirst == NULL)
			{
				pSegFirst = pSegCurrent = segments_pool_get();
			}
			else
			{
				pSegCurrent = pSegCurrent->pNext = segments_pool_get();
			}
			
			pSegCurrent->begin = i;
			
			i += 0;
			goto lbl_inSegment;
		}
	}
	
	return pSegCurrent;
	
	for (i = 0; i < width; i += 1)
	{
		lbl_inSegment:
		if (pImg[i] == value)
		{
			pSegCurrent->end = i;
			
			i += 0;
			goto lbl_outSegment;
		}
	}
	
	pSegCurrent->end = i;
	return pSegCurrent;
}

void ProcessFrame(uint8 const * const pRawImg)
{
	OSC_ERR err;
	enum EnBayerOrder enBayerOrder;
	uint16 row, col, width, height, greyCol, greyRow, greyWidth, greyHeight;
	uint32 pos;
	uint8 const threshold = 32;
	
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
	
	for (greyRow = 0; greyRow < greyHeight; greyRow += 1)
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




void process_frame_init() {
	segments_pool_init ();
}
