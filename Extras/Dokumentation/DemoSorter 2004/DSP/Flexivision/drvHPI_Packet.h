
/**
* @file 
* @brief Defines the packet structures and fields used for communication with the etrax host processor.
*
* Those fields must be defined in conjunction with the appropriate fields in the Etrax software.
*/

#ifndef _DRVHPI_PACKET_H_
#define _DRVHPI_PACKET_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <std.h>

// These are the possible data types:
#define HPIPT_CMD	 			1		///< A command message.
#define HPIPT_TEXT				2		///< A text message. Has no effect on the DSP but will be put on standard output by the Etrax.
#define HPIPT_IMAGE				3		///< An image message used to transfer images from the DSP to the etrax.

#define HPIPT_MAX_PAYLOAD		(1024*32-16)

/**
* The command structure that is used to transmit a
* command (e.g. LUT settings, image processing
* parameters etc.) to or from the DSP.
*/
typedef struct HPI_Data_Command {
	char		txtTxt[HPIPT_MAX_PAYLOAD];			///< The command in plain text.
} HPI_Data_Command, HPI_Data_Answer;

/**
* The structure that is used to transmit a text message
* to or from the DSP. The char count must not exceed
* HPIPT_MAX_PAYLOAD.
*/
typedef struct HPI_Data_Text {
	char		txtTxt[HPIPT_MAX_PAYLOAD];			///< The text.
} HPI_Data_Text;

/**
* The structure that is used to transmit a picture
* from the DSP to the etrax.
*/
typedef struct HPI_Data_Image{
	Uint32		imgChannel;				///< The channel on which the image should be sent.
	Uint8		imgData[1];				///< The image data. This will exceed the array's dimension.
} HPI_Data_Image;

/**
* This is the amount of data that is used to create a Image packet (in addition to 
* HPIPACKET_HEADERSIZE, PIC_HEADERSIZE and the picture pixeldata.
*/
#define HPIPACKET_IMGDATA_SIZE 4


/**
* The complete packet structure.
*
* The payload can be typecast to the corresponding
* message type.
*
* Besides its purpose to declare the packet's size, the Size field
* also indicate if the packet is valid.
* A value of 0xFFFFFFFF = (Int)-1 marks it as invalid
* and ready to overwrite. This is used for the message passing 
* mechanism between the etrax and the dsp (which takes places in
* the dsp's shared memory).
*/
typedef struct HPI_Packet
{
	Int32		Size;					///< The total size (in bytes) of this packet, including header and payload.
	Int32		DestAddr;				///< The destination address of this packet.
	Int32		SrcAddr;				///< The source address of this packet.
	Int32		Type;					///< This packet's payload type.
	

	union
	{	
		HPI_Data_Command	Command;	///< The command structure, if the message is a command.
		HPI_Data_Text		Text;		///< The text structure.
		HPI_Data_Image		Image;		///< The image structure.
	} Data;								///< Unifies all possible message types for easy access.
	
} HPI_Packet;

/**
* The packet's header size. Used for allocating a packet in memory (allocate (HPIPACKET_HEADERSIZE + payloadsize)
* bytes for the packet...).
*/
#define HPIPACKET_HEADERSIZE 16			

#ifdef __cplusplus
}
#endif

#endif

