
// PSNR_SSIM.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CPSNR_SSIMApp:
// See PSNR_SSIM.cpp for the implementation of this class
//

class CPSNR_SSIMApp : public CWinApp
{
public:
	CPSNR_SSIMApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CPSNR_SSIMApp theApp;