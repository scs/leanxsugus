// DlgText.cpp : implementation file
//

#include "stdafx.h"
#include "demo.h"
#include "DlgText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgText dialog


DlgText::DlgText(CWnd* pParent /*=NULL*/)
	: CDialog(DlgText::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgText)
	m_text = _T("");
	//}}AFX_DATA_INIT
	memset(&m_font,0,sizeof(m_font));
}


void DlgText::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgText)
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Control(pDX, IDCANCEL, m_canc);
	DDX_Control(pDX, ID_FONT, m_bfont);
	DDX_Text(pDX, IDC_EDIT1, m_text);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgText, CDialog)
	//{{AFX_MSG_MAP(DlgText)
	ON_BN_CLICKED(ID_FONT, OnFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgText message handlers

BOOL DlgText::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ok.SetIcon(IDI_G,BS_LEFT);
	m_canc.SetIcon(IDI_R,BS_LEFT);
	m_bfont.SetIcon(IDI_B,BS_LEFT);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgText::OnFont() 
{
	CFontDialog	dlg;
	memcpy(dlg.m_cf.lpLogFont, &m_font, sizeof(m_font));
	dlg.m_cf.rgbColors = m_color;
	if (dlg.DoModal()==IDOK){
		memcpy(&m_font,dlg.m_cf.lpLogFont, sizeof(m_font));
		m_color = dlg.GetColor();
	}
}
