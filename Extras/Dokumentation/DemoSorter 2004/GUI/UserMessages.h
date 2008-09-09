//UserMessages.h -------------- Only a header declaration file

#ifndef _USERMESSAGES_H_
#define _USERMESSAGES_H_


#define		GET_MSG					(WM_USER+1)
#define		SET_MSG					(WM_USER+2)
#define		GET_REPLY_MSG			(WM_USER+3)
#define		SET_REPLY_MSG			(WM_USER+4)
#define		RESET_MSG				(WM_USER+5)
#define		COUNTS_MSG				(WM_USER+6)
#define		COUNTS_REPLY_MSG		(WM_USER+7)

static const int NUMCOLORPROPS = 4;

struct Message_Get
{
	Message_Get(CString IDArg) : ID(IDArg){}
	Message_Get(){ ID = "not initialized";}
	CString ID;
};

struct Message_Set
{
	Message_Set(CString IDArg, double valueArg) : ID(IDArg),Value(valueArg){}
	Message_Set(){ Value = 0.0; ID = "not initialized";}
	CString ID;
	double Value;
};

struct Message_Get_Reply
{
	Message_Get_Reply(CString IDArg, double valueArg) : ID(IDArg),Value(valueArg){}
	Message_Get_Reply(){ ID = "not initialized", Value = 0.0;}
	CString ID;
	double Value;
};

struct Message_Set_Reply
{
	Message_Set_Reply(){ Success = false;}
	Message_Set_Reply(bool successArg) : Success(successArg){}
	bool Success;
	CString ID;
};

struct Message_Counts_Reply
{
	~Message_Counts_Reply()
	{
		delete [] countArr;
	}
	Message_Counts_Reply()
	{ 
		countArr = new double[NUMCOLORPROPS + 10];
		for (int i = 0; i<NUMCOLORPROPS; i++) countArr[i] = 0; 
	}
	double* countArr;
};

#endif