
// PSNR_SSIMDlg.h : header file
//

#pragma once


// CPSNR_SSIMDlg dialog
class CPSNR_SSIMDlg : public CDialogEx
{
// Construction
public:
	CPSNR_SSIMDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PSNR_SSIM_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON		m_hIcon;

	CString		m_strSrc;
	CString		m_strDst;
	int			m_iHeight;
	int			m_iWigth;
	CString		m_strFormat;
	FILE		*m_pSrcFile;
	FILE		*m_pDstFile;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1Open();
	afx_msg void OnBnClickedButton2Open();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();

	bool	InitPramance(int &iF, int &iY);
};
