
// PSNR_SSIMDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PSNR_SSIM.h"
#include "PSNR_SSIMDlg.h"
#include "afxdialogex.h"
#include <thread>
#include<conio.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define XCHG(type,a,b) { type t = a;  a =  b; b = t; }
#define X264_MIN(a,b) ( (a)<(b) ? (a) : (b) )
// CPSNR_SSIMDlg dialog



CPSNR_SSIMDlg::CPSNR_SSIMDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PSNR_SSIM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPSNR_SSIMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPSNR_SSIMDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1_Open, &CPSNR_SSIMDlg::OnBnClickedButton1Open)
	ON_BN_CLICKED(IDC_BUTTON2_Open, &CPSNR_SSIMDlg::OnBnClickedButton2Open)
	ON_BN_CLICKED(IDC_BUTTON3, &CPSNR_SSIMDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CPSNR_SSIMDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CPSNR_SSIMDlg message handlers

BOOL CPSNR_SSIMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	AllocConsole();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPSNR_SSIMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPSNR_SSIMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//GetSrcPath

void CPSNR_SSIMDlg::OnBnClickedButton1Open()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, _T("打开"), 0, OFN_HIDEREADONLY, "媒体文件|*.yuv");

	if (dlg.DoModal() == IDCANCEL) {
		return;
	}

	m_strSrc = dlg.GetPathName();//

	GetDlgItem(IDC_EDIT4_S)->SetWindowText(m_strSrc);
}

//GetDstPath
void CPSNR_SSIMDlg::OnBnClickedButton2Open()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, _T("打开"), 0, OFN_HIDEREADONLY, "媒体文件|*.yuv");

	if (dlg.DoModal() == IDCANCEL) {
		return;
	}

	m_strDst = dlg.GetPathName();//

	GetDlgItem(IDC_EDIT5_D)->SetWindowText(m_strDst);

}
static void ssim_4x4x2_core(const uint8_t *pix1, int stride1,
	const uint8_t *pix2, int stride2,
	int **sums)
{
	int x, y, z;
	for (z = 0; z < 2; z++)
	{
		uint32_t s1 = 0, s2 = 0, ss = 0, s12 = 0;
		for (y = 0; y < 4; y++)
			for (x = 0; x < 4; x++)
			{
				int a = pix1[x + y*stride1];
				int b = pix2[x + y*stride2];
				s1 += a;
				s2 += b;
				ss += a*a;
				ss += b*b;
				s12 += a*b;
			}
		sums[z][0] = s1;
		sums[z][1] = s2;
		sums[z][2] = ss;
		sums[z][3] = s12;
		pix1 += 4;
		pix2 += 4;
	}
}

static float ssim_end1(int s1, int s2, int ss, int s12)
{
	static const int ssim_c1 = (int)(.01*.01 * 255 * 255 * 64 + .5);
	static const int ssim_c2 = (int)(.03*.03 * 255 * 255 * 64 * 63 + .5);
	int vars = ss * 64 - s1*s1 - s2*s2;
	int covar = s12 * 64 - s1*s2;
	return (float)(2 * s1*s2 + ssim_c1) * (float)(2 * covar + ssim_c2)\
		/ ((float)(s1*s1 + s2*s2 + ssim_c1) * (float)(vars + ssim_c2));
}

static float ssim_end4(int **sum0, int **sum1, int width)
{
	int i;
	float ssim = 0.0;
	for (i = 0; i < width; i++)
		ssim += ssim_end1(sum0[i][0] + sum0[i + 1][0] + sum1[i][0] + sum1[i + 1][0],
			sum0[i][1] + sum0[i + 1][1] + sum1[i][1] + sum1[i + 1][1],
			sum0[i][2] + sum0[i + 1][2] + sum1[i][2] + sum1[i + 1][2],
			sum0[i][3] + sum0[i + 1][3] + sum1[i][3] + sum1[i + 1][3]);
	return ssim;
}
float x264_pixel_ssim_wxh(
	uint8_t *pix1, int stride1,
	uint8_t *pix2, int stride2,
	int width, int height)
{
	int x, y, z;
	float ssim = 0.0;
	int **sum0,**sum1;

	sum0 = new int*[4*(width / 4+3)];
	sum1 = new int*[4 * (width / 4 + 3)];
	for (int i = 0; i < 4*(width / 4 + 3); i++)
	{
		sum0[i] = new int[sizeof(int *)];
		sum1[i] = new int[sizeof(int *)];
	}

	width >>= 2;
	height >>= 2;
	z = 0;

	for (y = 1; y < height; y++)
	{
		for (; z <= y; z++)
		{
			XCHG(int**, sum0, sum1);
			for (x = 0; x < width; x += 2)
				ssim_4x4x2_core(&pix1[4 * (x + z*stride1)], stride1, &pix2[4 * (x + z*stride2)], stride2, &sum0[x]);
		}
		for (x = 0; x < width - 1; x += 4)
			ssim += ssim_end4(sum0 + x, sum1 + x, X264_MIN(4, width - x - 1));
	}

	for (int i = 0; i < 4 * (width / 4 + 3); i++)
	{
		delete[] sum0[i];
		delete[] sum1[i];
	}
	delete[] sum0;
	delete[] sum1;

	return ssim / ((width - 1) * (height - 1));
}

void SSIM(int iF, int iY, FILE *pSrc, FILE *pDst,int iHeight,int iWigth)
{
	if (pSrc == nullptr || pDst == nullptr)
		return;

	unsigned char *cSrc = nullptr;
	unsigned char *cDst = nullptr;

	cSrc = new unsigned char[iY];
	cDst = new unsigned char[iY];

	int N = 0, size = 0, i = 0;
	double *ypsnr = 0, yrmse = 0, diff = 0;

	for (;;) {
		if (1 != fread(cSrc, iY, 1, pSrc) || 1 != fread(cDst, iY, 1, pDst)) break;

		if (++N > size) {
			size += 0xffff;
			if (!(ypsnr = (double *)realloc(ypsnr, size * sizeof *ypsnr))) return;
		}
		ypsnr[N - 1] = x264_pixel_ssim_wxh(cSrc, iWigth, cDst, iWigth, iWigth, iHeight);

		_cprintf("SSIM::%.3f\n", ypsnr[N - 1]);
	}
}
void PSNR(int iF,int iY,FILE *pSrc,FILE *pDst)
{
	if (pSrc == nullptr || pDst == nullptr)
		return;

	unsigned char *cSrc = nullptr;
	unsigned char *cDst = nullptr;

	cSrc = new unsigned char[iY];
	cDst = new unsigned char[iY];

	int N = 0, size = 0,i = 0;
	double *ypsnr = 0, yrmse = 0, diff = 0;

	for (;;) {
		if (1 != fread(cSrc, iY, 1, pSrc) || 1 != fread(cDst, iY, 1, pDst)) break;

		if (++N > size) {
			size += 0xffff;
			if (!(ypsnr = (double *)realloc(ypsnr, size * sizeof *ypsnr))) return;
		}
		
		for (yrmse = 0, i = 0; i < iF; i += 1) {
			diff = cSrc[i] - cDst[i];
			yrmse += diff * diff;
		}
		ypsnr[N - 1] = yrmse ? 20 * (log10(255 / sqrt(yrmse / iF))) : 0;
		
		_cprintf("PSNR::%.3f\n", ypsnr[N - 1]);
	}

}
bool CPSNR_SSIMDlg::InitPramance(int &iF, int &iY)
{
	CString	strH, strW;

	GetDlgItem(IDC_EDIT1_H)->GetWindowText(strH);
	GetDlgItem(IDC_EDIT2_W)->GetWindowText(strW);
	GetDlgItem(IDC_EDIT3_F)->GetWindowText(m_strFormat);

	m_iWigth = _ttoi(strW);
	m_iHeight = _ttoi(strH);

	if (m_iWigth == 0 || m_iHeight == 0)
	{
		MessageBox("宽高设置失败!!");
		return false;
	}
	//
	iF = m_iHeight *m_iWigth;

	if (m_strFormat.CompareNoCase("yuv420") == 0)
	{
		iY = iF * 3 / 2;
	}
	if (m_strFormat.CompareNoCase("yuv422") == 0)
	{
		iY = iF * 2;
	}
	if (m_strFormat.CompareNoCase("yuv444") == 0)
	{
		iY = iF * 3;
	}
	if (iY == 0)
	{
		MessageBox("类型设置失败");
		return false;
	}
	//
	errno_t er = fopen_s(&m_pSrcFile, m_strSrc, "rb+");
	if (er != 0)
	{
		MessageBox("打开文件失败path:", m_strSrc);
		return false;
	}

	er = fopen_s(&m_pDstFile, m_strDst, "rb+");
	if (er != 0)
	{
		MessageBox("打开文件失败path:", m_strDst);
		return false;
	}
	return true;
}
//PSNR
void CPSNR_SSIMDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here

	int iF = 0, iY = 0;
	
	bool bre = InitPramance(iF, iY);

	std::thread th(PSNR, iF,iY, m_pDstFile, m_pSrcFile);
	th.join();
}

//SSIM
void CPSNR_SSIMDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	int iF = 0, iY = 0;

	bool bre = InitPramance(iF, iY);

	std::thread th(SSIM, iF, iY, m_pDstFile, m_pSrcFile,m_iHeight,m_iWigth);
	
	th.join();
}
