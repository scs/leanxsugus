
#include "tstEtraxDriver.h"

#ifdef _DEBUG
extern Bool hint;		///< import from drvHPI.c
#else
Bool hint = FALSE;
#endif


typedef struct HPITEST_Mailbox
{
	Byte	InBuffer[HPI_PACKETBUFFER_SIZE];	///< The input buffer (Etrax to DSP)
	Byte	OutBuffer[HPI_PACKETBUFFER_SIZE];	///< The output buffer (DSP to Etrax)
} HPITEST_Mailbox;

HPI_DevHandle 			hpi;
HPITEST_Mailbox 		* mbx = (Ptr)0x87E00000;
char					teststr[] = "123456789 This is a testtext.";



void EtraxDriverTest_SendMessage(HPI_Packet * packet);
void EtraxDriverTest_SendTextMessage(Int destaddr, Int srcaddr, const char * str);
void EtraxDriverTest_SendPing(Int destaddr, Int srcaddr);
void EtraxDriverTest_SendCommand(Int destaddr, Int srcaddr, Int cmd);
void EtraxDriverTest_ReceiveMessage(HPI_Packet * packet);
void EtraxDriverTest_tsk_funct();

void EtraxDriverTest()
{
	Int 					i;
	volatile int			vi;
	
	HPI_Packet				pckt_dsp;
	HPI_Packet				pckt_host;
	
	hpi = hpiOpen( 4 );
	
	assert (hpi != NULL);
	
	
	// *********************************************************
	//  Test 1
	// *********************************************************
	// Ping pong.
	
	// send ping message from the host
	EtraxDriverTest_SendPing( 10, 20 );	
	
	// receive with the DSP and send back pong
	hpiGetMessage( hpi, &pckt_dsp, SYS_FOREVER );
	assert( (pckt_dsp.Size == 16) && (pckt_dsp.DestAddr == 10) && (pckt_dsp.SrcAddr == 20) && (pckt_dsp.Type == HPIPT_PING) );
	pckt_dsp.SrcAddr = 10;
	pckt_dsp.DestAddr = 20;
	pckt_dsp.Type = HPIPT_PONG;
	hpiSendMessage( hpi, &pckt_dsp, 0 );
	
	// Receive pong with the host.
	EtraxDriverTest_ReceiveMessage( &pckt_host );
	assert( (pckt_host.Size == 16) && (pckt_host.DestAddr == 20) && (pckt_host.SrcAddr == 10) && (pckt_host.Type == HPIPT_PONG) );
	
	
	// *********************************************************
	//  Test 2
	// *********************************************************
	// Check the message queue capabilities
	
	// spawn the "host" thread
	TSK_create( (Fxn)EtraxDriverTest_tsk_funct, NULL );
	
	TSK_sleep(1);
	
	// Completely fill the queues; The host will need more time to read the messages,
	// so it will limit the throughput
	for (i=0; i<8; i++)
	{
	
		pckt_dsp.Size		= strlen(teststr + i) + 1 + 16;
		pckt_dsp.DestAddr	= i;
		pckt_dsp.SrcAddr 	= 20-i;
		pckt_dsp.Type		= HPIPT_TEXT;
		//strncpy( (char*)(pckt_dsp.Data.Text.txtTxt), teststr + i, 1);
		memcpy( (void*)(pckt_dsp.Data.Text.txtTxt), teststr + i, 10);
	
		hpiSendMessage( hpi, &pckt_dsp, SYS_FOREVER );
		
		vi++;
	}
	
	// the other direction:
	for (i=0; i<8; i++)
	{
		hpiGetMessage( hpi, &pckt_dsp, SYS_FOREVER );
		
		assert( (pckt_dsp.Type == HPIPT_CMD) && (pckt_dsp.SrcAddr == 30-i) && (pckt_dsp.DestAddr == 10+i) );
		assert( (pckt_dsp.Data.Command.cmdType == 128 + i ) );
		
		TSK_sleep(2);
	}
	
	LOG_printf( &trace, "HPItest: succesfully finished.");
	
	TSK_sleep(100);
	
	hpiClose( hpi );
}

// *************************************************************************

void EtraxDriverTest_tsk_funct()
{
	Int 		i;
	HPI_Packet 	packet;
	
	LOG_printf( &trace, "HPItest: Host Simulator started.");
	
	// *********************************************************
	//  Test 2
	// *********************************************************
	// DSP->HOST
	for (i=0; i<8; i++)
	{
		EtraxDriverTest_ReceiveMessage( &packet );
		assert( (packet.Type == HPIPT_TEXT) && (packet.SrcAddr == 20-i) && (packet.DestAddr == i) );
		assert( 0 == memcmp( packet.Data.Text.txtTxt, teststr + i, 10) );
		LOG_printf(&trace, "Sim: Message received.");
	}
	
	// HOST->DSP
	for (i=0; i<8; i++)
	{
		EtraxDriverTest_SendCommand( 10+i, 30-i, 128+i );		
	}
	
	LOG_printf( &trace, "HPItest: Host Simulator succesfully finished.");
}

/**
* Simulates the ETRAX's behaviour with writing a message to the common
* mailbox and then triggering a DSPINT interrupt.
*/
void EtraxDriverTest_SendMessage(HPI_Packet * packet)
{
	volatile HPI_Packet 	* mbxpacket;
	
	mbxpacket = (HPI_Packet*)(mbx->InBuffer);
	
	while (mbxpacket->Size != -1) TSK_sleep(1);
	
	memcpy((Ptr)mbxpacket, (Ptr)packet, packet->Size);
	IRQ_set(IRQ_EVT_DSPINT);
}

// *************************************************************************

void EtraxDriverTest_SendTextMessage(Int destaddr, Int srcaddr, const char * str)
{
	HPI_Packet 			packet;
	
	packet.Size		= strlen(str) + 1 + 16;
	packet.DestAddr	= destaddr;
	packet.SrcAddr 	= srcaddr;
	packet.Type		= HPIPT_TEXT;
	memcpy( (char*)(packet.Data.Text.txtTxt), str, 10);
	
	EtraxDriverTest_SendMessage( &packet );
}

// *************************************************************************

void EtraxDriverTest_SendPing(Int destaddr, Int srcaddr)
{
	HPI_Packet 			packet;
	
	packet.Size		= 16;
	packet.DestAddr	= destaddr;
	packet.SrcAddr 	= srcaddr;
	packet.Type		= HPIPT_PING;	
	
	EtraxDriverTest_SendMessage( &packet );
}

// *************************************************************************

void EtraxDriverTest_SendCommand(Int destaddr, Int srcaddr, Int cmd)
{
	HPI_Packet 			packet;
	
	packet.Size		= 20;
	packet.DestAddr	= destaddr;
	packet.SrcAddr 	= srcaddr;
	packet.Type		= HPIPT_CMD;	
	packet.Data.Command.cmdType = cmd;
	
	EtraxDriverTest_SendMessage( &packet );
}

// *************************************************************************

void EtraxDriverTest_ReceiveMessage(HPI_Packet * packet)
{
	volatile HPI_Packet 	* mbxpacket;
	
	mbxpacket = (HPI_Packet*)(mbx->OutBuffer);
	
	// Wait to receive message
	while ( ! hint )
		TSK_sleep(1);
	
	// invalidate hint.
	hint = FALSE;
			
	// simulate some delay
	TSK_sleep(1);	
	
	memcpy((Ptr)packet, (Ptr)mbxpacket, mbxpacket->Size);
	mbxpacket->Size = -1;
	mbxpacket->DestAddr = -1;
	mbxpacket->SrcAddr  = -1;
}



