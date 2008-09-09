// VisionProperty.cpp: implementation of the CVisionProperty class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DemoSorterGUI.h"
#include "VisionProperty.h"
#include "DSPCommand.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVisionProperty::CVisionProperty(CString IDArg, int valueArg, CDSPComm* commArg) : CProperty(IDArg, valueArg, commArg)
{

}

CVisionProperty::CVisionProperty() : CProperty()
{

}

CVisionProperty::~CVisionProperty()
{

}

bool CVisionProperty::PerformRead( double & dValue )
{
	CDSPCommand readCmd(comm);

	CString testCString; //dient zum vergleichen mit Strings aus der Liste, was weggeschnitten werden muss
	testCString.Format("prop Classifier.%s = ",ID); 

	CString testStr; //ist temporärer String, 

	CString cmdCString; //eigentlicher Kommando String

	cmdCString.Format("prop get Classifier." + ID); //

	if (readCmd.process(cmdCString))
	{
		//TRACE("Command processing successfull!!!!!\n");
		//TRACE("%d lines received.\n", readCmd.GetNumRecvLines());
		for (int i = 0; i < readCmd.GetNumRecvLines(); i++)
		{
			//TRACE("Line %d is: %s\n", i, readCmd.GetResponse(i));
			if(readCmd.GetResponse(i).Find(testCString,0) != -1) //we found substring "prop Classifier."
			{
				
				testStr = readCmd.GetResponse(i); //temporäre Kopie des ausgewählten Strings aus dem Vektor
				
				testStr.TrimLeft(testCString); //schneide alles bis auf Wert weg
				
				//TRACE("String after cutting: %s.\n", testStr);

				const char* number = LPCTSTR(testStr);

				dValue = atof(number);

				//TRACE("number extracted: %f.\n", dValue);

				update(dValue);

			}
			else
			{
				TRACE("Could not find desired substring in desired string %d.\nUpdate() could not be achieved!\n", i);
			}
		}
	}
	else
	{
		TRACE("command processing failed!!!!!!!\n");
	}
	
	return true;
}


bool CVisionProperty::PerformWrite( const double dValue )
{
	CDSPCommand writeCmd(comm);
	
	CString cmdCString;
	cmdCString.Format("prop set Classifier." + ID + " %f", dValue);

	if (writeCmd.process(cmdCString))
	{
		//TRACE("Command processing successfull!!!!!\n");
		//TRACE("%d lines received.\n", writeCmd.GetNumRecvLines());
		
		//for (int i = 0; i < writeCmd.GetNumRecvLines(); i++)
		//{
			//TRACE("Line %d is: %s\n", i, writeCmd.GetResponse(i));
		//}
		//delete writeCmd;

		update(dValue);

		return true; //should always be the case, because we dont expect anything than "done" line
	}
	else
	{
		TRACE("command processing failed!!!!!!!\n");

		return false;
	}

	return false;

}
