// ModeProperty.cpp: implementation of the CModeProperty class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DemoSorterGUI.h"
#include "ModeProperty.h"
#include "DSPCommand.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModeProperty::CModeProperty(CString IDArg, int valueArg, CDSPComm* commArg) : CProperty(IDArg, valueArg, commArg)
{

}

CModeProperty::CModeProperty() : CProperty()
{

}

CModeProperty::~CModeProperty()
{

}

bool CModeProperty::PerformRead( double & modeValued )
{
	CDSPCommand readCmd(comm);
	
	CString testCString; //dient zum vergleichen mit Strings aus der Liste, was weggeschnitten werden muss
	testCString.Format("%s = ",ID); 

	CString testStr; //ist temporärer String, 

	CString cmdCString("mode");

	if (readCmd.process(cmdCString))
	{
		//TRACE("Command processing successfull!!!!!\n");
		//TRACE("%d lines received.\n", readCmd.GetNumRecvLines());
		for (int i = 0; i < readCmd.GetNumRecvLines(); i++)
		{
			//TRACE("Line %d is: %s\n", i, readCmd.GetResponse(i));
			if(readCmd.GetResponse(i).Find(testCString,0) != -1) //we found substring "mode"
			{
				
				testStr = readCmd.GetResponse(i); //temporäre Kopie des ausgewählten Strings aus dem Vektor
				
				testStr.TrimLeft(testCString); //schneide alles bis auf Wert weg
				
				//TRACE("String after cutting: %s.\n", testStr);

				if (testStr == "idle")
					modeValued = 0;
				else if (testStr == "processing")
					modeValued = 1;
				else if (testStr == "calibration")
					modeValued = 2;
				else if (testStr == "service")
					modeValued = 3;

				update(modeValued);
			}
		}

		return true;
	}
	else
	{
		TRACE("command processing failed!!!!!!!\n");

		return false;
	}


	return false;
}


bool CModeProperty::PerformWrite( double modeValued )
{
	CDSPCommand writeCmd(comm);
	
	CString cmdCString;

	switch(int(modeValued))
	{
	case 0:	cmdCString.Format("mode idle");
			break;
	case 1: cmdCString.Format("mode processing");
			break;
	case 2:	cmdCString.Format("mode calibration");
			break;
	case 3: cmdCString.Format("mode service");
			break;
	default:
			TRACE("mode type not valid.\n");
			break;
	}
	
	if (writeCmd.process(cmdCString))
	{
		//TRACE("Command processing successfull!!!!!\n");
		//TRACE("%d lines received.\n", writeCmd.GetNumRecvLines());
		//for (int i = 0; i < writeCmd.GetNumRecvLines(); i++)
		//{
			//TRACE("Line %d is: %s\n", i, writeCmd.GetResponse(i));
		//}
		update(modeValued);

		return true;
	}
	else
	{
		TRACE("command processing failed!!!!!!!\n");
		
		return false;
	}

	return false;

}
