﻿// OARConvWin.cpp : アプリケーションのクラス動作を定義します。
//

#include  "stdafx.h"

#include  "OARConvWin.h"
#include  "MainFrm.h"

#include  "Tools.h"
#include  "Dx9.h"
#include  "Graph.h"
#include  "WinTools.h"

#include  "BREPView.h"
#include  "BREPDoc.h"
#include  "BREPFrame.h"
#include  "BREP_MFCLib.h"

#include  "ShowInfoDLG.h"
#include  "SetParamDLG.h"
#include  "OutFormatDLG.h"
#include  "ObjectsListDLG.h"

#include  "tar32api.h"

#ifdef WIN64
#pragma comment(lib, "tar64.lib")
#else
#pragma comment(lib, "tar32.lib")
#endif


using namespace jbxl;
using namespace jbxwl;


// COARConvWinApp
BEGIN_MESSAGE_MAP(COARConvWinApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &COARConvWinApp::OnAppAbout)
	// 標準のファイル基本ドキュメント コマンド
	//ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// 標準の印刷セットアップ コマンド
	//ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	//ON_COMMAND(ID_FILE_OPEN, OnFileOpenBrep)
	//ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_COMMAND(ID_FILE_OPEN, &COARConvWinApp::OnFileOpen)
	ON_COMMAND(ID_FOLDER_OPEN, &COARConvWinApp::OnFolderOpen)
	ON_COMMAND(ID_CONVERT_DATA, &COARConvWinApp::OnConvertData)
	ON_COMMAND(ID_FILE_QUICK, &COARConvWinApp::OnFileOpenQuick)
	ON_COMMAND(ID_FOLDER_QUICK, &COARConvWinApp::OnFolderOpenQuick)
	ON_COMMAND(ID_LOG_WINDOW, &COARConvWinApp::OnLogWindow)
	ON_COMMAND(ID_OUTPUT_FORMAT, &COARConvWinApp::OnOutFormatDialog)
	ON_COMMAND(ID_SETTING_DIALOG, &COARConvWinApp::OnSettingDialog)
	ON_COMMAND(ID_OBJ_LIST, &COARConvWinApp::OnObjectsList)
END_MESSAGE_MAP()


// COARConvWinApp コンストラクション
COARConvWinApp::COARConvWinApp()
{
	pDocTemplLOG  = NULL;
	pDocTemplBREP = NULL;

	pMainFrame    = NULL;
	pLogFrame	  = NULL;
	pLogDoc		  = NULL;
	aboutBox	  = NULL;
	objListBox    = NULL;

	memset(&windowSize, -1, sizeof(RECT));
	appParam.init();

	char* prgfld = GetProgramFolderA();
	homeFolder = make_Buffer_bystr(prgfld);
	::free(prgfld);

	assetsFolder = dup_Buffer(homeFolder);
	cat_s2Buffer(OART_DEFAULT_AST_DIR, &assetsFolder);

	comDecomp = dup_Buffer(homeFolder);
	cat_s2Buffer(OART_JP2_DECOMP_COM, &comDecomp);

	hasData = false;
	isConverting = false;
}


//
COARConvWinApp::~COARConvWinApp()
{
	DEBUG_INFO("DESTRUCTOR: COARConvWinApp START\n");
	
	free_Buffer(&homeFolder);
	free_Buffer(&assetsFolder);
	free_Buffer(&comDecomp);

	appParam.saveConfigFile();
	appParam.free();
	oarTool.free();

	deleteNull(objListBox);
	deleteNull(aboutBox);
	Dx9ReleaseInterface();

	DEBUG_INFO("DESTRUCTOR: COARConvWinApp END\n");
}


//
COARConvWinApp theApp;


// COARConvWinApp 初期化
BOOL  COARConvWinApp::InitInstance()
{
	CWinApp::InitInstance();

	AfxEnableControlContainer();
	Enable3dControlsStatic();	// MFC と静的にリンクしている場合にはここを呼び出してください．

	SetRegistryKey(_T("OARConvWin by NSL"));	
	LoadStdProfileSettings(4);  // 標準の INI ファイルのオプションをロードします (MRU を含む)

	///////////////////////////////////////////////////////////////////////////////
	//
	CMultiDocTemplate* pDocTemplate;

	// ログ ウィンドウ
	pDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CLogWndDoc),
		RUNTIME_CLASS(CLogWndFrame),
		RUNTIME_CLASS(CLogWndView));
	if (!pDocTemplate) return FALSE;
	AddDocTemplate(pDocTemplate);
	pDocTemplLOG = pDocTemplate;

	// BREP ウィンドウ 
	pDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CBREPDoc),
		RUNTIME_CLASS(CBREPFrame), 
		RUNTIME_CLASS(CBREPView));

	if (!pDocTemplate) return FALSE;
	AddDocTemplate(pDocTemplate);
	pDocTemplBREP = pDocTemplate;
	
	///////////////////////////////////////////////////////////////////////////////
	// メイン MDI フレーム ウィンドウ
	pMainFrame = new CMainFrame;
	pMainFrame->pApp = this;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME)) return FALSE;
	m_pMainWnd = pMainFrame;
	m_pMainWnd->DragAcceptFiles(TRUE);

	///////////////////////////////////////////////////////////////////////////////
	// Locale
	setSystemLocale();

	///////////////////////////////////////////////////////////////////////////////
	// ウィンドウサイズ
	int wx = OARCONV_WND_XSIZE;
	int wy = OARCONV_WND_YSIZE;
	int rx = GetSystemMetrics(SM_CXSCREEN);			// X方向の解像度
	int ry = GetSystemMetrics(SM_CYSCREEN) - 14;	// Y方向の解像度（-ツールバー）

	// Default Size
	windowSize.left   = rx/2 - wx/2;
	windowSize.right  = wx;
	windowSize.top    = ry/2 - wy/2;
	windowSize.bottom = wy;

	// read  windowSize
	appParam.readWindowSize(&windowSize);

	int sx, sy;
	if (windowSize.left<=rx) {
		sx = windowSize.left;
		wx = windowSize.right;
		if (wx<=0 || wx>=rx) wx = OARCONV_WND_XSIZE; 
	}
	else {
		sx = rx/2 - OARCONV_WND_XSIZE/2;
	}
	if (windowSize.top<=ry) {
		sy = windowSize.top;
		wy = windowSize.bottom;
		if (wy<=0 || wy>=ry) wy = OARCONV_WND_YSIZE;
	}
	else {
		sy = ry/2 - OARCONV_WND_YSIZE/2;
	}
	pMainFrame->SetWindowPos(NULL, sx, sy, wx, wy, 0);

	///////////////////////////////////////////////////////////////////////////////
	// メイン ウィンドウの表示と更新
	pMainFrame->ShowWindow(SW_SHOW);
	pMainFrame->UpdateWindow();

	///////////////////////////////////////////////////////////////////////////////
	// DirectX9 の初期化
	BOOL rslt = Dx9CreateInterface(this);
	if (!rslt) {
		MessageBoxDLG(IDS_STR_ERROR, IDS_STR_FAIL_GET_DX9DEV, MB_OK, m_pMainWnd); // "DirectXの初期化に失敗しました"
		return FALSE;
	}

	///////////////////////////////////////////////////////////////////////////////
	// 初期化
	appParam.readConfigFile();
	DebugMode = appParam.debugMode;
	//
	updateMenuBar();

	return TRUE;
	// この後，メッセージループに入る．
}


// CExTextFrame のディストラクト時のコールバック関数
void  COARConvWinApp::FrameDestructor(CExTextFrame* frm)
{
	// ログ ウィンドウ
	if (frm==pLogFrame) {
		DEBUG_INFO("COARConvWinApp::FrameDestructor(): STOP LOG FRAME");
		pLogFrame = NULL;
	}
	
	updateMenuBar();
	return;
}


// CExTextView のディストラクト時のコールバック関数
void  COARConvWinApp::ViewDestructor(CExTextView* view)
{
	// ログ ウィンドウ
	if (view==pLogFrame->pView) {
		DEBUG_INFO("COARConvWinApp::ViewDestructor(): STOP LOG VIEW");
		ClearGlobalTextDocument();
		pLogDoc = NULL;
	}

	updateMenuBar();
	return;
}


/////////////////////////////////////////////////////////////////////////////
// アプリケーションのバージョン情報で使われる CAboutDlg ダイアログ
//
// pfrm->pView->TimerStart() の値の影響でウィンドウが多数表示される場合，
// DoModal() が表示されない現象が発生するので，その対応処理.
void  COARConvWinApp::OnAppAbout()
{
	if (aboutBox==NULL) aboutBox = new CMessageBoxDLG(IDD_ABOUTBOX, NULL, m_pMainWnd);
	if (aboutBox!=NULL) aboutBox->Display();
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////
// COARConvWinApp メッセージ ハンドラ
//

//
void  COARConvWinApp::OnFileOpen()
{
	CString fname = EasyGetOpenFileName(_T("Open OAR File"), m_pMainWnd->m_hWnd);
	if (fname.IsEmpty()) return;

	bool ret = fileOpen(fname);
	if (ret) showOARInfoDLG();

	return;
}


//
void  COARConvWinApp::OnFolderOpen()
{
	CString crntd = getOARFolder();
	CString foldr = EasyGetOpenFolderName(crntd, _T("Open OAR Folder"), m_pMainWnd->m_hWnd);
	if (foldr.IsEmpty()) return;

	bool ret = folderOpenOAR(foldr);
	if (ret) showOARInfoDLG();

	return;
}


//
void  COARConvWinApp::OnConvertData()
{
	convertAllData();
	return;
}


//
void  COARConvWinApp::OnFileOpenQuick()
{
	CString fname = EasyGetOpenFileName(_T("Open OAR File"), m_pMainWnd->m_hWnd);
	if (fname.IsEmpty()) return;

	bool ret = fileOpen(fname);
	if (!ret) return;

	convertAllData();
	return;
}


//
void  COARConvWinApp::OnFolderOpenQuick()
{
	CString crntd = getOARFolder();
	CString foldr = EasyGetOpenFolderName(crntd, _T("Open OAR Folder"), m_pMainWnd->m_hWnd);
	if (foldr.IsEmpty()) return;

	bool ret = folderOpenOAR(foldr);
	if (!ret) return;

	convertAllData();
	return;
}


//
void  COARConvWinApp::OnObjectsList()
{
	if (objListBox==NULL) {
		tList* objlist = oarTool.GetObjectsList();
		if (objlist!=NULL) {
			objListBox = new CObjectsListDLG(objlist, this, m_pMainWnd);
			if (objListBox==NULL) return;
			objListBox->Display();
		}
	}
	
	updateMenuBar();
	return;
}


//
void  COARConvWinApp::OnLogWindow()
{
	if (pLogFrame==NULL) {
		pLogFrame = ExecLogWnd(pDocTemplLOG, _T("Log Window"), this);
		pLogDoc = pLogFrame->pDoc;
		SetGlobalTextDocument(pLogDoc);
	}

	updateMenuBar();
	return;
}


void  COARConvWinApp::OnOutFormatDialog()
{
	COutFormatDLG* setdlg = new COutFormatDLG(&appParam, m_pMainWnd);
	if (setdlg == NULL) return;
	setdlg->DoModal();
	setdlg->getParameters(&appParam);
	delete (setdlg);

    oarTool.SetEngine(appParam.outputEngine);
/*
	if (appParam.outputEngine == JBXL_3D_ENGINE_UNITY) {
        
		oarTool.setUnity(true);
		oarTool.setUE(false);
	}
	if (appParam.outputEngine == JBXL_3D_ENGINE_UE) {
		oarTool.setUnity(false);
		oarTool.setUE(true);
	}
	else {
		oarTool.setUnity(false);
		oarTool.setUE(false);
	}*/

	char* outdir = ts2mbs(getBaseFolder() + appParam.prefixOUT + getOARName());
	oarTool.ChangePathInfo(NULL, outdir, NULL);
	::free(outdir);

	appParam.saveConfigFile();
	return;
}


void  COARConvWinApp::OnSettingDialog()
{
	CSetParamDLG* setdlg = new CSetParamDLG(&appParam, m_pMainWnd);
	if (setdlg==NULL) return;
	setdlg->DoModal();
	setdlg->getParameters(&appParam);
	delete (setdlg);

	char* outdir = ts2mbs(getBaseFolder() + appParam.prefixOUT + getOARName());
	oarTool.ChangePathInfo(NULL, outdir, NULL);
	::free(outdir);

	DebugMode = appParam.debugMode;
	appParam.saveConfigFile();
	return;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//

bool  COARConvWinApp::fileOpen(CString fname)
{
	bool ret;

	CString fn = get_file_name_t((LPCTSTR)fname);
	if (!fn.Compare(_T("archive.xml"))) {
		CString path = get_file_path_t(fname);
		ret = folderOpenOAR(path);
	}
	else {
		ret = fileOpenOAR(fname);
	}
	return ret;
}


//
// OARファイルをオープンする．
//
bool  COARConvWinApp::fileOpenOAR(CString fname)
{
	hasData = false;
	updateMenuBar();
	updateStatusBar(_T(""));

	// Check
	char* fp = ts2mbs((LPCTSTR)fname);		// 要 free
	int ret  = TarCheckArchive(fp, 0);
	::free(fp);
	if (!ret) {
		MessageBoxDLG(IDS_STR_ERROR, IDS_STR_ERR_OPEN_FILE, MB_OK, pMainFrame);
		return false;
	}

	CString path = get_file_path_t(fname);	// パス名
	CString file = get_file_name_t(fname);
	file = cut_file_extension_t(file);		// 拡張子なしファイル名

	CString oarf = path + appParam.prefixOAR + file;
	if (file_exist_t((LPCTSTR)oarf)) tunlink((LPCTSTR)oarf);

	// Extract
	CString mode = _T("-zxvf ") + fname + _T(" -o ") + oarf;
	char* md = ts2mbs((LPCTSTR)mode);
	ret = Tar(pMainFrame->m_hWnd, md, NULL, 0);
	::free(md);
	if (ret) {
		MessageBoxDLG(IDS_STR_ERROR, IDS_STR_ERR_EXTRACT, MB_OK, pMainFrame);
		return false;
	}

	// Output Folder
	CString outf = path + appParam.prefixOUT + file;
	setOARName((LPCTSTR)file);
	setBaseFolder((LPCTSTR)path);
	setOARFolder((LPCTSTR)oarf);
	setOutFolder((LPCTSTR)outf);
	appParam.saveConfigFile();

	////////////////////////////////////////////////////////////////////
	char* oardir = ts2mbs(getOARFolder());
	char* outdir = ts2mbs(getOutFolder());
	oarTool.free();
	oarTool.init();
	oarTool.SetPathInfo(appParam.outputFormat, oardir, outdir, (char*)assetsFolder.buf);
 	::free(oardir);
	::free(outdir);

	bool chk = oarTool.GetDataInfo();
	if (!chk) {
		MessageBoxDLG(IDS_STR_ERROR, IDS_STR_NOT_OAR, MB_OK, pMainFrame);
		return false;
	}

	hasData = true;
	updateMenuBar();
	updateStatusBar(getOARFolder());
	//
	char* fn = ts2mbs(fname);
	PRINT_MESG("fileOpenOAR: File is opened %s\n", fn);
	free(fn);

	return chk;
}


//
// OARフォルダをオープンする．
//
bool  COARConvWinApp::folderOpenOAR(CString folder)
{
	hasData = false;
	updateMenuBar();
	updateStatusBar(_T(""));

	CString oarf = folder;
	if (oarf.Right(1)==_T("\\")) oarf = oarf.Left(oarf.GetLength()-1);

	int len = appParam.prefixOAR.GetLength();
	CString outf;
	CString prnt = get_file_path_t(oarf);
	CString dirn = get_file_name_t(oarf);
	CString topd = dirn.Left(len);
	if (!topd.Compare(appParam.prefixOAR)) {
		dirn = dirn.Right(dirn.GetLength() - len);
	}
	outf = prnt + appParam.prefixOUT + dirn;

	setOARName((LPCTSTR)dirn);		// dirn は OARの名前になっているはず
	setBaseFolder((LPCTSTR)prnt);
	setOARFolder((LPCTSTR)oarf);
	setOutFolder((LPCTSTR)outf);
	appParam.saveConfigFile();

	////////////////////////////////////////////////////////////////////
	char* oardir = ts2mbs(getOARFolder());
	char* outdir = ts2mbs(getOutFolder());
	oarTool.free();
	oarTool.init();
	oarTool.SetPathInfo(appParam.outputFormat, oardir, outdir, (char*)assetsFolder.buf);
	::free(oardir);
	::free(outdir);

	bool chk = oarTool.GetDataInfo();
	if (!chk) {
		MessageBoxDLG(IDS_STR_ERROR, IDS_STR_NOT_OAR, MB_OK, pMainFrame);
		return false;
	}

	hasData = true;
	updateMenuBar();
	updateStatusBar(getOARFolder());
	//
	char* fn = ts2mbs(folder);
	PRINT_MESG("folderOpenOAR: Folder is opened %s\n", fn);
	free(fn);

	return chk;
}


//
void  COARConvWinApp::convertAllData()
{
	isConverting = true;
	updateMenuBar();

	oarTool.MakeOutputFolder(appParam.outputFormat);;

	int num = convertAllFiles();
	if (num>=0) {
		CString format;
		CString mesg;
		format.LoadString(IDS_STR_CONVERT_NUM);
		mesg.Format(format, num);
		MessageBoxDLG(IDS_STR_CNFRM, mesg, MB_OK, m_pMainWnd);
	}
	else {
		MessageBoxDLG(IDS_STR_CANCEL, IDS_STR_CANCEL_TRNS_DATA, MB_OK, m_pMainWnd);
	}

	isConverting = false;
	updateMenuBar();

	return;
}



/////////////////////////////////////////////////////////////////////////////////////////
// Convert
//

//
int   COARConvWinApp::convertAllFiles()
{
	int strtnum = appParam.startNum;
	int stopnum = appParam.stopNum;

	// 処理データ数の推測（Terrainは除く）
	int prognum = oarTool.objectsNum;
	if (stopnum>0) prognum = Min(prognum, stopnum);
	if (strtnum<1) strtnum = 1;
	prognum -= strtnum + 1;
	//
	oarTool.SetShift(appParam.xShift, appParam.yShift, appParam.zShift);

	//////////////////////////////////////////////////////////////////////////////
	// Convert
	int num = 0;
	if (appParam.outputTerrain) {
		CMessageBoxDLG* mbox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_CONV_TERRAIN, m_pMainWnd);
		if (mbox!=NULL) mbox->Display();
		//
		oarTool.ReadTerrainData();
		oarTool.SetTerrainTextureScale(appParam.terrainScale);
		num = oarTool.GenerateTerrainDataFile(appParam.outputFormat);

		if (mbox!=NULL) delete mbox;
	}
	//
	if (stopnum!=0) {
		CProgressBarDLG* progress = new CProgressBarDLG(IDD_PROGBAR, _T(""), TRUE);
		if (progress!=NULL) {
			if      (appParam.outputFormat==JBXL_3D_FORMAT_DAE) progress->SetTitle("Convert to DAE Files");
			else if (appParam.outputFormat==JBXL_3D_FORMAT_OBJ) progress->SetTitle("Convert to OBJ Files");
			else                                                progress->SetTitle("Convert to STL Files");
			progress->Start(prognum);
			SetGlobalCounter(progress);
		}
		//
		num = oarTool.GenerateObjectsDataFile(appParam.outputFormat, strtnum, stopnum, true, (char*)comDecomp.buf);
		//
		if (progress!=NULL) {
			progress->PutFill();
			delete progress;
			ClearGlobalCounter();
		}
	}
	return num;
}


void  COARConvWinApp::convertSelectedData(int selectedNums, int* selectedObjs)
{
	isConverting = true;
	updateMenuBar();
	oarTool.MakeOutputFolder(appParam.outputFormat);

	//
	int num = convertSelectedFiles(selectedNums, selectedObjs);
	if (num>=0) {
		CString format;
		CString mesg;
		format.LoadString(IDS_STR_CONVERT_NUM);
		mesg.Format(format, num);
		MessageBoxDLG(IDS_STR_CNFRM, mesg, MB_OK, m_pMainWnd);
	}
	else {
		MessageBoxDLG(IDS_STR_CANCEL, IDS_STR_CANCEL_TRNS_DATA, MB_OK, m_pMainWnd);
	}

	isConverting = false;
	updateMenuBar();

	return;
}


//
int   COARConvWinApp::convertSelectedFiles(int selectedNums, int* selectedObjs)
{
	if (selectedNums<=0 || selectedObjs==NULL) return 0;

	DebugMode   = appParam.debugMode;
	int prognum = selectedNums;
	oarTool.SetShift(appParam.xShift, appParam.yShift, appParam.zShift);

	//////////////////////////////////////////////////////////////////////////////
	// Convert
	int num = 0;
	CProgressBarDLG* progress = new CProgressBarDLG(_T(""), TRUE);
	if (progress!=NULL) {
		if      (appParam.outputFormat == JBXL_3D_FORMAT_DAE) progress->SetTitle("Convert to DAE Files");
		else if (appParam.outputFormat == JBXL_3D_FORMAT_OBJ) progress->SetTitle("Convert to OBJ Files");
		else                                                  progress->SetTitle("Convert to STL Files");
		progress->Start(prognum);
		SetGlobalCounter(progress);
	}
	//
	num = oarTool.GenerateSelectedDataFile(appParam.outputFormat, selectedNums, selectedObjs, true, (char*)comDecomp.buf);
	//
	if (progress!=NULL) {
		progress->PutFill();
		delete progress;
		ClearGlobalCounter();
	}
	
	return num;
}


void  COARConvWinApp::convertOneData(int index, BOOL outputDae)
{
	isConverting = true;
	updateMenuBar();
	if (outputDae) oarTool.MakeOutputFolder(true);
	else           oarTool.MakeOutputFolder(false);

	//
	int num = convertOneFile(index, outputDae);
	if (num>=0) {
		CString format;
		CString mesg;
		format.LoadString(IDS_STR_CONVERT_NUM);
		mesg.Format(format, num);
		MessageBoxDLG(IDS_STR_CNFRM, mesg, MB_OK, m_pMainWnd);
	}
	else {
		MessageBoxDLG(IDS_STR_CANCEL, IDS_STR_CANCEL_TRNS_DATA, MB_OK, m_pMainWnd);
	}

	isConverting = false;
	updateMenuBar();

	return;
}


//
int   COARConvWinApp::convertOneFile(int index, BOOL outputDae)
{
	DebugMode = appParam.debugMode;
	oarTool.SetShift(appParam.xShift, appParam.yShift, appParam.zShift);

	//////////////////////////////////////////////////////////////////////////////
	// Convert
	int num = oarTool.GenerateObjectsDataFile(appParam.outputFormat, index, index, true, (char*)comDecomp.buf);
	
	return num;
}



/////////////////////////////////////////////////////////////////////////////////////////
//

void  COARConvWinApp::solidOpenBrep(BREP_SOLID* solid, LPCTSTR title, int num)
{
	if (solid==NULL) return;

	CExFrame* pfrm = CreateDocFrmView(pDocTemplBREP, this);
	if (pfrm!=NULL) {
		((CBREPDoc*)(pfrm->pDoc))->Solid   = solid;
		((CBREPDoc*)(pfrm->pDoc))->DataNum = num; 
		((CBREPDoc*)(pfrm->pDoc))->WinApp  = this;
		int ret = ExecDocFrmView(pfrm);
		if (pfrm->pDoc->pView!=NULL) pfrm->pDoc->pView->SetTitle(title);

		ExecDocFrmViewError(m_pMainWnd->m_hWnd, ret); 
		if (ret==0) pfrm->pView->TimerStart();
	}

	return;
}


void  COARConvWinApp::showOARInfoDLG()
{
	CShowInfoDLG* shwdlg = new CShowInfoDLG(oarTool, m_pMainWnd);
	if (shwdlg==NULL) return;
	shwdlg->DoModal();
	delete (shwdlg);

	return;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tools
//

void  COARConvWinApp::updateMenuBar(CMenu* menu)
{
	if (menu==NULL) {
		if (pMainFrame==NULL) return;
		menu = pMainFrame->GetMenu();
		if (menu==NULL) return;
	}

	// default
	menu->EnableMenuItem(ID_FILE_OPEN,    MF_BYCOMMAND | MF_ENABLED);
	menu->EnableMenuItem(ID_FOLDER_OPEN,  MF_BYCOMMAND | MF_ENABLED);
	menu->EnableMenuItem(ID_FILE_QUICK,   MF_BYCOMMAND | MF_ENABLED);
	menu->EnableMenuItem(ID_FOLDER_QUICK, MF_BYCOMMAND | MF_ENABLED);
	menu->EnableMenuItem(ID_CONVERT_DATA, MF_BYCOMMAND | MF_ENABLED);
	menu->EnableMenuItem(ID_OBJ_LIST,     MF_BYCOMMAND | MF_ENABLED);

	//
	if (isConverting) {
		menu->EnableMenuItem(ID_FILE_OPEN,    MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_FOLDER_OPEN,  MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_FILE_QUICK,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_FOLDER_QUICK, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_CONVERT_DATA, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_OBJ_LIST,     MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);	
	}
	else {
		if (!hasData) {
			menu->EnableMenuItem(ID_CONVERT_DATA, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
			menu->EnableMenuItem(ID_OBJ_LIST,    MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
	}

	if (objListBox!=NULL) {
		menu->EnableMenuItem(ID_FILE_OPEN,    MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_FOLDER_OPEN,  MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_FILE_QUICK,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_FOLDER_QUICK, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_CONVERT_DATA, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_OBJ_LIST,     MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	return;
}


void  COARConvWinApp::updateStatusBar(CString path)
{
	if (pMainFrame==NULL) return;

    CString prefix;
    if      (appParam.outputFormat == JBXL_3D_FORMAT_DAE)   prefix = _T("DAE: ");
    else if (appParam.outputFormat == JBXL_3D_FORMAT_OBJ)   prefix = _T("OBJ: ");
    else if (appParam.outputFormat == JBXL_3D_FORMAT_STL)   prefix = _T("STL: ");

    if      (appParam.outputEngine == JBXL_3D_ENGINE_UNITY) prefix += _T("UNITY: ");
    else if (appParam.outputEngine == JBXL_3D_ENGINE_UE)    prefix += _T("UE: ");

	CString mesg = prefix + _T("OAR Path: ") + path;
	pMainFrame->SetStausBarText(mesg);

	return;
}
