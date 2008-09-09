// CountsProperty.cpp: implementation of the CCountsProperty class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DemoSorterGUI.h"
#include "CountsProperty.h"
#include "UserMessages.h"
#include "DSPCommand.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CCounts::CCounts(CDSPComm* commArg)
{
	comm = commArg;
	values = new double[NUMCOLORPROPS + 10];
}

CCounts::CCounts()
{
	comm = NULL;
	values = new double[NUMCOLORPROPS + 10];
}

CCounts::~CCounts()
{
	delete [] values;
}

bool CCounts::Read( double dValue [] )
{
	CDSPCommand readCmd(comm);

	//CString testCString; //dient zum vergleichen mit Strings aus der Liste, was weggeschnitten werden muss
	//testCString.Format("prop enum"); 

	CString testStr; //ist temporärer String, 

	CString cmdCString; //eigentlicher Kommando String

	cmdCString.Format("prop enum"); //

	int countLoops = 0;

	if (readCmd.process(cmdCString))
	{
		//TRACE("Command processing successfull!!!!!\n");
		//TRACE("%d lines received.\n", readCmd.GetNumRecvLines());
		for (int i = 0; i < readCmd.GetNumRecvLines(); i++)
		{
			//TRACE("Line %d is: %s\n", i, readCmd.GetResponse(i));
			if(readCmd.GetResponse(i).Find(".Count") != -1) //we found substring ".Count"
			{
				//int s = i;
				//for (int countLoops = 0; countLoops < NUMCOLORPROPS; countLoops++)
				//{
					testStr = readCmd.GetResponse(i); //temporäre Kopie des ausgewählten Strings aus dem Vektor
					//TRACE(testStr);

					int n = testStr.Delete(0, 31);
	
					const char* number = LPCTSTR(testStr);

					dValue[countLoops] = atof(number);
					
					values[countLoops] = dValue[countLoops];
					
					//TRACE("Zeile Nr.%d: %s", i, testStr);
						
					countLoops++;

					//s += 3;
				//}
				//return true;
			}
			else
			{
				//TRACE("Could not find desired substring in desired string %d.\nUpdate() could not be achieved!\n", i);
			}
		}
	}
	else
	{
		TRACE("command processing failed!!!!!!!\n");
	}
	
	return true;
}



bool CCounts::Reset()
{
	CDSPCommand writeCmd(comm);
	
	CString cmdCString;

	for (int i = 0; i < NUMCOLORPROPS; i++)
	{
		cmdCString.Format("prop set Classifier.Color%d.Count 0", i);

		if (!writeCmd.process(cmdCString))
			return false;
	}				

		
	return true;
}
