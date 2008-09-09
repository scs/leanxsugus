#include "DSPComm.h"

#ifndef _PROPERTY_H_
#define _PROPERTY_H_

/*
available commands for the DSP:

		dsp>> help
->		dsp>> mode [idle/processing/service/calibration]
		dsp>> chan [open/close/snap/event/enum] nChan strComp.strPort [nDropRate/nEvent]
->		dsp>> prop [set/get/enum] strComp.strProperty nValue
->		dsp>> stats [reset]
		dsp>> cam [setlevels/send] [red green blue/sendstr]
		dsp>> ping
		dsp>> status
		dsp>> profile
		dsp>> log strLogtext
		dsp>> debug [on/off/maxfps/frametime] [fps/frametime_us]
		dsp>> service [jets]
		dsp>> test nTestnum
		dsp>> testcopy
		dsp>> Done

dsp<< mode [idle/processing]			-> ModeProperty
dsp<< prop [set/get]					-> VisionProperty

(dsp<< stats)

*/

class CProperty{

protected:
	CDSPComm* comm; //Referenz auf statisches Kommunikationsobjekt der Klasse CCommunication

	CString ID; //hat feste formatierung: yellow.sat / red.lum / machine.state / etc...
				//Es gibt 8 Farben:			yellow, orange, red, pink, purple, blue, green, brown
				//Farbe hat Eigenschaften:	sat, hue, lum, strictness, counts1, counts2, target
				//Es gibt nur eine Maschine: machine
				//hat die Eigenschaften:	state, isConnected, updateTime	
	double value; //--> doublre

public:
	CProperty::CProperty(CString IDArg, int valueArg,  CDSPComm* comm);
	CProperty();

	~CProperty();

	bool Read( double & dValue );
	bool Write( const double dValue );
	void update(const double dValue);

protected:
	virtual bool PerformRead( double & dValue ) = 0;
	virtual bool PerformWrite( const double dValue ) = 0;

};

#endif