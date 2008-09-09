// ModifyPropertyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "visiontest.h"
#include "ModifyPropertyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyPropertyDlg dialog


CModifyPropertyDlg::CModifyPropertyDlg(CWnd* pParent, CVisProperty * prop)
	: CDialog(CModifyPropertyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyPropertyDlg)
	m_strPropertyEdit = _T("");
	//}}AFX_DATA_INIT

	m_pProperty = prop;
}


void CModifyPropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyPropertyDlg)
	DDX_Text(pDX, IDC_PROPERTYEDIT, m_strPropertyEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyPropertyDlg, CDialog)
	//{{AFX_MSG_MAP(CModifyPropertyDlg)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyPropertyDlg message handlers

void CModifyPropertyDlg::OnOK() 
{
	float f;
	int i;

	// Validate the entered value
	UpdateData( TRUE );
	if ( sscanf( m_strPropertyEdit, "%f", &f ) != 1)
	{
		if ( sscanf( m_strPropertyEdit, "%d", &i ) != 1)
		{
			// Display error messagebox and try again
			MessageBox( "Please enter a correct value!", "ERROR", MB_ICONERROR );

			CEdit * edit;
			edit = (CEdit*)GetDlgItem( IDC_PROPERTYEDIT );
			edit->SetSel( 0xFFFF0000 );
			edit->SetFocus();

			return;
		}
		else
		{
			f = (float)i;
		}
	}

	m_pProperty->SetFloatValue( f );

	CDialog::OnOK();
	
}

void CModifyPropertyDlg::OnCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

BOOL CModifyPropertyDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CString str;

	// Set the dialog's title
	str.Format("Property %s.%s", m_pProperty->GetComponent()->GetName(), m_pProperty->GetName() );
	SetWindowText( str );

	// Write the initial value of the property to the edit field
	float f;
	m_pProperty->GetFloatValue( f );
	str.Format( "%f", f );
	m_strPropertyEdit = str;
	UpdateData(FALSE);

	// Set the focus
	CEdit * edit;
	edit = (CEdit*)GetDlgItem( IDC_PROPERTYEDIT );
	edit->SetSel( 0xFFFF0000 );
	edit->SetFocus();
	
	return FALSE;
	/*
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
				  */
}
