// RS232C-TEST01.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "RS232C-TEST01.h"

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;								// 現在のインターフェイス
TCHAR szTitle[MAX_LOADSTRING];					// タイトル バーのテキスト
TCHAR szWindowClass[MAX_LOADSTRING];			// メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Dialog1(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Dialog2(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Dialog3(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Dialog4(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Dialog5(HWND, UINT, WPARAM, LPARAM);

//*************************************************************************
//******          このアプリケーションでのグローバル変数           ********
//*************************************************************************
//        記憶するメモリ
int rs_comport;        //シリアルのポート番号
int rs_speed;          //シリアルのボーレート
int rs_dtbit;          //シリアルのデータービット
int rs_pybit;          //シリアルのパリティビット
int rs_spbit;          //シリアルのストップビット
int rs_flow;           //シリアルフロー制御 0=無し 1=Xon/off 2=DTR
//        記憶しないメモリ
wchar_t currentdirectory_dt[MAX_PATH];
wchar_t sIniFile[MAX_PATH];
wchar_t sAppName[MAX_PATH];

struct wc_buf{
 wchar_t s[256];       //11
};
wc_buf wcbuffer[256];   //画面表示した内容を記憶


//*************************************************************************
//******          ｉｎｉファイル読込処理                             ********
//*************************************************************************
void inifile_read(HWND hWnd)
{
	memset(currentdirectory_dt,NULL,sizeof(currentdirectory_dt));
	GetCurrentDirectory(MAX_PATH,currentdirectory_dt);   //カレントデレクトリの取得
	
	memset(sIniFile,NULL,sizeof(sIniFile));
	wcscpy_s(sIniFile,wcslen(currentdirectory_dt)+1,currentdirectory_dt);
	wcscat_s(sIniFile,wcslen(sIniFile)+2,TEXT("\\"));
	wcscat_s(sIniFile,wcslen(sIniFile)+18,TEXT("rs232c-test01.ini"));

	memset(sAppName,NULL,sizeof(sAppName));
	wsprintf(sAppName,TEXT("rs232c-test01"));

	rs_comport = GetPrivateProfileInt(sAppName, TEXT("rs_comport"), 3, sIniFile);
	rs_speed = GetPrivateProfileInt(sAppName, TEXT("rs_speed"), 19200, sIniFile);
	rs_dtbit = GetPrivateProfileInt(sAppName, TEXT("rs_dtbit"), 8, sIniFile);
	rs_pybit = GetPrivateProfileInt(sAppName, TEXT("rs_pybit"), 0, sIniFile);
	rs_spbit = GetPrivateProfileInt(sAppName, TEXT("rs_spbit"), 0, sIniFile);
	rs_flow = GetPrivateProfileInt(sAppName, TEXT("rs_flow"), 0, sIniFile);

}

//*************************************************************************
//******          ｉｎｉファイル書込処理                             ********
//*************************************************************************
void inifile_write(HWND hWnd)
{
	wchar_t asc_dt[MAX_PATH];
	SetCurrentDirectory(currentdirectory_dt);   //カレントデレクトリの設定

	memset(sIniFile,NULL,sizeof(sIniFile));
	wcscpy_s(sIniFile,wcslen(currentdirectory_dt)+1,currentdirectory_dt);
	wcscat_s(sIniFile,wcslen(sIniFile)+2,TEXT("\\"));
	wcscat_s(sIniFile,wcslen(sIniFile)+18,TEXT("rs232c-test01.ini"));

	memset(sAppName,NULL,sizeof(sAppName));
	wsprintf(sAppName,TEXT("rs232c-test01"));

	memset(asc_dt,NULL,sizeof(asc_dt));
	wsprintf(asc_dt,TEXT("%d"),rs_comport);
    WritePrivateProfileString(sAppName, TEXT("rs_comport"), asc_dt, sIniFile);
	memset(asc_dt,NULL,sizeof(asc_dt));
	wsprintf(asc_dt,TEXT("%d"),rs_speed);
    WritePrivateProfileString(sAppName, TEXT("rs_speed"), asc_dt, sIniFile);
	memset(asc_dt,NULL,sizeof(asc_dt));
	wsprintf(asc_dt,TEXT("%d"),rs_dtbit);
    WritePrivateProfileString(sAppName, TEXT("rs_dtbit"), asc_dt, sIniFile);
	memset(asc_dt,NULL,sizeof(asc_dt));
	wsprintf(asc_dt,TEXT("%d"),rs_spbit);
    WritePrivateProfileString(sAppName, TEXT("rs_pybit"), asc_dt, sIniFile);
	memset(asc_dt,NULL,sizeof(asc_dt));
	wsprintf(asc_dt,TEXT("%d"),rs_spbit);
    WritePrivateProfileString(sAppName, TEXT("rs_spbit"), asc_dt, sIniFile);
	memset(asc_dt,NULL,sizeof(asc_dt));
	wsprintf(asc_dt,TEXT("%d"),rs_flow);
    WritePrivateProfileString(sAppName, TEXT("rs_flow"), asc_dt, sIniFile);
}

//*************************************************************************
//******          ＲＳ２３２での定義     　                        ********
//*************************************************************************
#define inbuffersize	100    //rs232c：100以外にすると大量の文字を受けるとこける
#define outbuffersize 100   //rs232c：
HANDLE hCom1,hThread;
BOOL bReading=FALSE;
DWORD Read1Thread (LPDWORD /*lpdwParam1*/);
DWORD id;
COMMCONFIG cc;
COMMPROP cp;
DCB  ch1_dcb;           //16*4+4*2+8=80
COMMTIMEOUTS ch1_rstime;//4*5=20
// ****** オーバーラップ構造体 *******
OVERLAPPED ovlpd_ch1r;
OVERLAPPED ovlpd_ch1w;
// ****** エラー情報 *******
DWORD lpErrorsCH1; /* エラー情報 */
COMSTAT lpStatCH1; /* ポートの状態に関する情報 */
DWORD lpErrorsCH1_lach; /* エラー情報（ラッチ用） */
// ****** 受信データ *******
char inbuff[inbuffersize*2];
wchar_t inbuff2[inbuffersize*2];
int py=0;
HWND hWnd2;


// *************************************************************
// **　ＤＣＢとＴＩＭＥの設定　メモリから内容を書き込む **
// *************************************************************
int dcbtim1_set(HWND hWnd)
{
	wchar_t s[64];
	int a;

	// DCBの読み出し
	ch1_dcb.DCBlength = sizeof(DCB);
	if(GetCommState(hCom1, &ch1_dcb)==0)
	{
		a=GetLastError();
		wsprintf(s,TEXT("DCB Error No.%d"),a);
		MessageBox(hWnd,s,TEXT("GetCommState"),MB_OK);
		return 1;
	}
	// DCBの設定
	ch1_dcb.fBinary 	  = 1;
	ch1_dcb.BaudRate      = rs_speed;
	ch1_dcb.ByteSize      = rs_dtbit;
	ch1_dcb.fParity		  = 0;
	ch1_dcb.Parity        = rs_pybit;
	ch1_dcb.StopBits      = rs_spbit;
	if(rs_flow==0)
	{
		ch1_dcb.fRtsControl   = 0;
		ch1_dcb.fOutX 			= 0;
		ch1_dcb.fInX 			= 0;
		ch1_dcb.fOutxCtsFlow = 0;
	}
	if(rs_flow==1)
	{
		ch1_dcb.fRtsControl   = 0;
		ch1_dcb.fOutX 			= 1;
		ch1_dcb.fInX 			= 1;
		ch1_dcb.fOutxCtsFlow = 0;
	}
	if(rs_flow==2)
	{
		ch1_dcb.fRtsControl   = 2;
		ch1_dcb.fOutX 			= 0;
		ch1_dcb.fInX 			= 0;
		ch1_dcb.fOutxCtsFlow = 1;
	}
	ch1_dcb.fOutxDsrFlow = 0;
	ch1_dcb.fDsrSensitivity 	= 0;
	ch1_dcb.fTXContinueOnXoff 	= 0;
	ch1_dcb.fErrorChar 	= 0;
	ch1_dcb.fNull 			= 0;
	ch1_dcb.fAbortOnError = 0;
	ch1_dcb.fDtrControl   = 1;
	ch1_dcb.fDummy2       = 0;
	ch1_dcb.XonLim        = 2048;
	ch1_dcb.XoffLim       = 512;
	ch1_dcb.XonChar       = 0x11;
	ch1_dcb.XoffChar      = 0x13;
	ch1_dcb.ErrorChar     = 0;
	ch1_dcb.EofChar       = 0;
	ch1_dcb.EvtChar       = 0x0d;
	// DCBの書き込み
	ch1_dcb.DCBlength = sizeof(DCB);
	if(SetCommState(hCom1, &ch1_dcb) == 0) 
	{
		a=GetLastError();
		wsprintf(s,TEXT("DCB Error No.%d"),a);
		MessageBox(hWnd,s,TEXT("SetCommState"),MB_OK);
		return 1;
	}

	// Timerの読み出し
	if(GetCommTimeouts(hCom1, &ch1_rstime) == 0)
	{
		a=GetLastError();
		wsprintf(s,TEXT("Timeout Error No.%d"),a);
		MessageBox(hWnd,s,TEXT("GetCommTimeouts"),MB_OK);
	   	return 1;
	}
	// Timerの設定
	ch1_rstime.ReadIntervalTimeout         = 100;
	ch1_rstime.ReadTotalTimeoutMultiplier  = 2;
	ch1_rstime.ReadTotalTimeoutConstant    = 100;
	ch1_rstime.WriteTotalTimeoutMultiplier = 10;
	ch1_rstime.WriteTotalTimeoutConstant   = 1000;
	// Timerの書き込み
	if(SetCommTimeouts(hCom1, &ch1_rstime) == 0)
	{
		a=GetLastError();
		wsprintf(s,TEXT("Timeout Error No.%d"),a);
		MessageBox(hWnd,s,TEXT("SetCommTimeouts"),MB_OK);
	   	return 1;
	}
	return 0;

}

// ******************************************************
// **       　ＤＣＢから設定をメモリへ書き込む         **
// ******************************************************
int dcbmem_set(HWND hWnd)
{
	wchar_t s[64];
	int a;

	// DCBの読み出し
	ch1_dcb.DCBlength = sizeof(DCB);
	if(GetCommState(hCom1, &ch1_dcb)==0)
	{
		a=GetLastError();
		wsprintf(s,TEXT("DCB Error No.%d"),a);
		MessageBox(hWnd,s,TEXT("GetCommState"),MB_OK);
		return 1;
	}

	// DCBの設定
	rs_speed = ch1_dcb.BaudRate;
	rs_dtbit = ch1_dcb.ByteSize;
	rs_pybit = ch1_dcb.Parity;
	rs_spbit = ch1_dcb.StopBits;
	if(ch1_dcb.fRtsControl==0)
	{
		if(ch1_dcb.fOutX==0 && ch1_dcb.fInX==0)
			rs_flow=0;
		else
			rs_flow=1;
	}
	else
	{
		rs_flow=2;
	}
	return 0;

}

//*************************************************************************
//******          ＲＳ２３２スレッド処理 　                        ********
//*************************************************************************
DWORD Read1Thread (LPDWORD /*lpdwParam1*/){
	DWORD dwEvent,rs_read;
	DWORD endtime;
//	DWORD rs_err;
//	DWORD writefile_suu;
//	DWORD writefile_err;
//	int rsdtbf;
//	int rsdtbf2;
	DWORD dwEvtMask;
	HDC hdc;
	int len;
	RECT rect1;

	dwEvtMask=0;
	if(true)
		dwEvtMask|=EV_RXCHAR;
	else
		dwEvtMask|=EV_RXFLAG;
	SetCommMask (hCom1,dwEvtMask);        //EV_RXCHAR
	ovlpd_ch1r.hEvent = CreateEvent(NULL,true,false,NULL);
	while (bReading) 
	{
		//******* イベント待機　********
		if(WaitCommEvent (hCom1, &dwEvent, NULL))
		{
      		//******* エラーとステータス情報取得　********
			ClearCommError(hCom1,&lpErrorsCH1,&lpStatCH1);
			lpErrorsCH1_lach|=lpErrorsCH1;
			if(((dwEvent & EV_RXCHAR) || (dwEvent & EV_RXFLAG)) && lpStatCH1.cbInQue)
			{
				//******* 非同期式　読込シーケンス　********
				memset(inbuff,NULL,inbuffersize*2);
				if(ReadFile(hCom1,inbuff,inbuffersize,&rs_read,&ovlpd_ch1r)==0)
				{
					rs_read=0;
					if(GetLastError()==ERROR_IO_PENDING)
					{
						endtime = GetTickCount()+1000;
						while(GetOverlappedResult(hCom1,&ovlpd_ch1r,&rs_read,FALSE)==0)
						{
							if(GetTickCount() > endtime)
							{
								break;
							}
						}
					}
					else
					{
						rs_read=0;
					}
				}
				if(rs_read)
				{
					memset(inbuff2,NULL,sizeof(inbuff2));
					len=MultiByteToWideChar(CP_THREAD_ACP,MB_PRECOMPOSED,inbuff,-1,NULL,NULL);					
					if(len>0)
					{
						MultiByteToWideChar(CP_THREAD_ACP,MB_PRECOMPOSED,inbuff,-1,inbuff2,len);					
					
						hdc=GetDC(hWnd2);
						GetClientRect(hWnd2,&rect1);
						TextOut(hdc,0,py*16,TEXT("　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"),50);
						TextOut(hdc,0,py*16,inbuff2,wcslen(inbuff2));
						memset(wcbuffer[py].s,NULL,256);
						wsprintf(wcbuffer[py].s,inbuff2);
						py++;
						if((py*16)>(rect1.bottom-1))
							py=0;
						else
						{
							TextOut(hdc,0,py*16,TEXT("　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"),50);
							memset(wcbuffer[py].s,NULL,256);
						}
						ReleaseDC(hWnd2,hdc);
					}
				}
			}
		}
		ResetEvent(ovlpd_ch1r.hEvent);
	}
	PurgeComm(hCom1,PURGE_RXCLEAR);
	return 0;
}


//*************************************************************************
//******          ＲＳ２３２Ｃ　ＯＰＥＮ処理                       ********
//*************************************************************************
int rs1_open(HWND hWnd){
	wchar_t port_no[32];
	wchar_t s[64];
	int a;
	hWnd2=hWnd;

	if(rs_comport>0 && rs_comport<256)
	{
		memset(port_no,NULL,sizeof(port_no));
		wsprintf(port_no,TEXT("\\\\.\\COM%d"),rs_comport);
	}
	else
	{
		MessageBox(hWnd,TEXT("No. Error"),TEXT("Port_No."),MB_OK);
		return 1;  //ポートの番号が正しくありません
	}
	hCom1 = CreateFile(port_no       // ポートの名前
					, GENERIC_READ | GENERIC_WRITE  // Read/Write
					, 0              // 共有しない
					, NULL           // セキュリティ属性デフォルト
					, OPEN_EXISTING  // 既存ファイル
					, FILE_FLAG_OVERLAPPED // 非同期 I/O を許す
					, NULL);
  	if (hCom1 == INVALID_HANDLE_VALUE)
	{
		a=GetLastError();
		wsprintf(s,TEXT("Open Error No.%d"),a);
		MessageBox(hWnd,s,TEXT("CreateFile"),MB_OK);
		return 1;  //ポートがオープンしません
  	}

	SetupComm(hCom1,inbuffersize,outbuffersize);
	/* get comm properties */
	cp.wPacketLength = sizeof(COMMPROP);
	GetCommProperties (hCom1, &cp);

	if(dcbtim1_set(hWnd)==1)
	{
		CloseHandle (hCom1);
		return 1; //ポートの設定はできません
	}

  	if (bReading == FALSE){
    	hThread=CreateThread (NULL, //def security
                          	0,    //def stack size
         						(LPTHREAD_START_ROUTINE)Read1Thread,
                          	NULL, //param to pass to thread
                          	0,
                          	&id);
		if (hThread == NULL){
			CloseHandle (hCom1);
			//MessageBox(NULL,"CreateThread(1)で失敗しました。","DebugはWinで行こう",MB_OK);
			a=GetLastError();
			wsprintf(s,TEXT("Thread Error No.%d"),a);
			MessageBox(hWnd,s,TEXT("CreateThread"),MB_OK);
			return 1; //スレッドの設定はできません
		}
	}
	ovlpd_ch1r.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
	bReading = TRUE;
	/* Buffer Clear */
	PurgeComm( hCom1, PURGE_TXABORT | PURGE_RXABORT |
					  PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
	return 0;
}

//*************************************************************************
//******          ＲＳ２３２Ｃ　ＣＬＯＳＥ処理                     ********
//*************************************************************************
void rs1_close(){
	bReading = FALSE;
	DWORD dwEvtMask;

	dwEvtMask=0;
	if(true)
		dwEvtMask|=EV_RXCHAR;
	else
		dwEvtMask|=EV_RXFLAG;
	SetCommMask (hCom1,dwEvtMask);
	while (true)
	{
		GetExitCodeThread(hThread, &id);
		if (id == STILL_ACTIVE)
			continue;
		else
			break;
	} /* end while (no error reading thread exit code) */
	// Buffer Clear
	PurgeComm( hCom1,PURGE_TXABORT | PURGE_RXABORT |
					 PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
	CloseHandle (hThread);
	CloseHandle(hCom1);
}

//***************************************************************
//******          ＲＳ２３２Ｃの設定                     ********
//***************************************************************
void rs1_setting(HWND hWnd)
{
	wchar_t s[64];
	int a,e;
	DWORD lrc;

	lrc=sizeof(COMMCONFIG); //lrc=0;
	GetCommConfig(hCom1, &cc, &lrc);
	cc.dwSize = lrc;
	cc.dcb=ch1_dcb;
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("COM%d"),rs_comport);
	a = CommConfigDialog(s,hWnd, &cc);
	if (!a)
	{
		e = GetLastError();
    	if(e != 1223)
		{
			wsprintf(s,TEXT("CommConfigDialogのErr番号は%d:cc.size=%d"),e,lrc);
			MessageBox(hWnd,s,TEXT("CommConfigDialog"),MB_OK);
			return;
		}
	}
	/* write new settings */
	cc.dwSize = lrc;
	a = SetCommState (hCom1, &cc.dcb);
	if (!a){
		e = GetLastError();
		wsprintf(s,TEXT("SetCommStateのErr番号は%d:cc.size=%d"),e,lrc);
		MessageBox(hWnd,s,TEXT("SetCommState"),MB_OK);
		return;
	}

	ch1_dcb=cc.dcb;
	dcbmem_set(hWnd);
}

// ******************* 画面表示 ***********************
void print_pro(HWND hWnd,HDC hdc,PAINTSTRUCT *ps)
{
	int i;
	RECT rect1;

	GetClientRect(hWnd,&rect1);
	for(i=0;i<256;i++)
	{
		TextOut(hdc,0,i*16,wcbuffer[i].s,wcslen(wcbuffer[i].s));
		if((i*16)>(rect1.bottom-1))
			break;
	}

}


//
//  関数: _tWinMain()
//
//  目的: メイン関数を登録します。
//           
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: ここにコードを挿入してください。
	MSG msg;
	HACCEL hAccelTable;

	// グローバル文字列を初期化しています。
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_RS232CTEST01, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RS232CTEST01));

	// メイン メッセージ ループ:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
//  コメント:
//
//    この関数および使い方は、'RegisterClassEx' 関数が追加された
//    Windows 95 より前の Win32 システムと互換させる場合にのみ必要です。
//    アプリケーションが、関連付けられた
//    正しい形式の小さいアイコンを取得できるようにするには、
//    この関数を呼び出してください。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RS232CTEST01));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_RS232CTEST01);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

	/*画面サイズ取得*/
	int width,height;
	HDC hdc;
	hdc=GetDC(NULL);
	width = GetDeviceCaps(hdc, HORZRES );
	height =GetDeviceCaps(hdc, VERTRES );
	ReleaseDC(NULL,hdc);

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
						(width-560)/2,(height-600)/2,560,600,
						//CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
						NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:  メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
//	int a;
//	wchar_t s[64];

	switch (message)
	{
	case WM_CREATE:
		inifile_read(hWnd);
		if(rs1_open(hWnd))
		{
			MessageBox(hWnd,TEXT("RS22CがOPENしません"),TEXT("RS22C-TEST01"),MB_OK);
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 選択されたメニューの解析:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_PORT:
			rs1_close();
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, Dialog1);
			if(rs1_open(hWnd))
			{
				MessageBox(hWnd,TEXT("RS22CがOPENしません"),TEXT("RS22C-TEST01"),MB_OK);
			}
			break;
		case IDM_RSSET:
			rs1_setting(hWnd);
			break;
		case IDM_DCB:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, Dialog2);
			break;
		case IDM_MONITOR:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hWnd, Dialog3);
			break;
		case IDM_INSEND:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG4), hWnd, Dialog4);
			break;
		case IDM_SETSEND:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG5), hWnd, Dialog5);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
     	print_pro(hWnd,hdc,&ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		//MessageBox(hWnd,TEXT("test"),TEXT("test"),MB_OK);
		inifile_write(hWnd);
		rs1_close();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


//*************************************************************************
//******          ＣＯＭＰＯＲＴの設定処理                         ********
//*************************************************************************

//************　初期化　***************
void Dialog1_int(HWND hDlg)
{
   HWND hwndcombo;

   //レジストリからＣＯＭ番号を取得
   LONG Ret1;
   HKEY hKey1;
   DWORD dwIndex;
   TCHAR szName[256];
   DWORD dwNameSize;
   DWORD dwType;
   wchar_t cbuff[] = TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM");
   wchar_t dataBuff[256];
   DWORD dwSize;
   wchar_t s[32];

	// ************ Port設定comboBox ***************
	hwndcombo=GetDlgItem(hDlg, IDC_COMBO1);
	SendMessage(hwndcombo, CB_RESETCONTENT, 0, 0);

	//キーをオープンしてハンドルを得る
	Ret1 = RegOpenKeyEx(HKEY_LOCAL_MACHINE,cbuff,NULL,KEY_QUERY_VALUE,&hKey1);
	for (dwIndex = 0;;dwIndex++)
	{
		memset(szName,NULL,sizeof(szName));
		dwNameSize=sizeof(szName)-1;
		//インデックスから名前を取得
       	Ret1 = RegEnumValue(hKey1,dwIndex,szName,&dwNameSize,NULL,&dwType,NULL,NULL);
		//ERROR_NO_MORE_ITEMSまで繰り返す
		if(Ret1 == ERROR_NO_MORE_ITEMS ) break;
		switch (dwType)
		{
			case REG_SZ:
				//名前からデーターを取得する
				memset(dataBuff,NULL,sizeof(dataBuff));
				dwSize=sizeof(dataBuff)-1;
				Ret1 = RegQueryValueEx(hKey1, szName, NULL, &dwType, (LPBYTE)dataBuff, &dwSize);
				if(memcmp(dataBuff,TEXT("COM"),3)==0)
				{
					SendMessage(hwndcombo,CB_ADDSTRING,0,(LPARAM)dataBuff);
				}
				break;
			default:
				break;
		}
	}
	Ret1 = RegCloseKey(hKey1);

	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("COM%d"),rs_comport);
	SendMessage(hwndcombo, WM_SETTEXT, 0,(LPARAM)s);
}

//************　設定登録　***************
void Dialog1_out(HWND hDlg)
{
   wchar_t s[32];

	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg, IDC_COMBO1,s,8);
	rs_comport=_wtoi(&s[3]);

}

// Ｐｏｒｔ番号の設定　Ｄｉａｌｏｇ。
INT_PTR CALLBACK Dialog1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		Dialog1_int(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON1:
			Dialog1_out(hDlg);
		case IDC_BUTTON2:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg,TRUE);
		break;
	case WM_DESTROY:
		break;
	}
	return (INT_PTR)FALSE;
}

//*************************************************************************
//******          ＤＣＢ＆ＴＩＭＥの設定処理                       ********
//*************************************************************************

//************　初期化　***************
void Dialog2_int(HWND hDlg)
{
	wchar_t s[32];

	GetCommState(hCom1, &ch1_dcb);
	GetCommTimeouts(hCom1, &ch1_rstime);

	// ************ CheckBox ***************
	CheckRadioButton(hDlg, IDC_CHECKBOX1,IDC_CHECKBOX11,NULL);
	if(ch1_dcb.fBinary)  // 0= N  1=Y
		CheckRadioButton(hDlg, IDC_CHECKBOX1,IDC_CHECKBOX1,IDC_CHECKBOX1);
	if(ch1_dcb.fOutxCtsFlow)  // 0= N  1=Y
		CheckRadioButton(hDlg, IDC_CHECKBOX2,IDC_CHECKBOX2,IDC_CHECKBOX2);
	if(ch1_dcb.fOutxDsrFlow)  // 0= N  1=Y
		CheckRadioButton(hDlg, IDC_CHECKBOX3,IDC_CHECKBOX2,IDC_CHECKBOX3);
	if(ch1_dcb.fDsrSensitivity)  // 0= N  1=Y
		CheckRadioButton(hDlg, IDC_CHECKBOX4,IDC_CHECKBOX2,IDC_CHECKBOX4);
	if(ch1_dcb.fTXContinueOnXoff)  // 0= N  1=Y
		CheckRadioButton(hDlg, IDC_CHECKBOX5,IDC_CHECKBOX2,IDC_CHECKBOX5);
	if(ch1_dcb.fOutX)  // 0= N  1=Y
		CheckRadioButton(hDlg, IDC_CHECKBOX6,IDC_CHECKBOX2,IDC_CHECKBOX6);
	if(ch1_dcb.fInX)  // 0= N  1=Y
		CheckRadioButton(hDlg, IDC_CHECKBOX7,IDC_CHECKBOX2,IDC_CHECKBOX7);
	if(ch1_dcb.fErrorChar)  // 0= N  1=Y
		CheckRadioButton(hDlg, IDC_CHECKBOX8,IDC_CHECKBOX2,IDC_CHECKBOX8);
	if(ch1_dcb.fNull)  // 0= N  1=Y
		CheckRadioButton(hDlg, IDC_CHECKBOX9,IDC_CHECKBOX2,IDC_CHECKBOX9);
	if(ch1_dcb.fAbortOnError)  // 0= N  1=Y
		CheckRadioButton(hDlg, IDC_CHECKBOX10,IDC_CHECKBOX2,IDC_CHECKBOX10);
	if(ch1_dcb.fParity)  // 0= N  1=Y
		CheckRadioButton(hDlg, IDC_CHECKBOX11,IDC_CHECKBOX2,IDC_CHECKBOX11);

	// ************ editBox ***************
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_dcb.fDtrControl);
	SetDlgItemText(hDlg, IDC_EDIT1, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_dcb.fRtsControl);
	SetDlgItemText(hDlg, IDC_EDIT2, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_dcb.fDummy2);
	SetDlgItemText(hDlg, IDC_EDIT3, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_dcb.XonLim);
	SetDlgItemText(hDlg, IDC_EDIT4, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_dcb.XoffLim);
	SetDlgItemText(hDlg, IDC_EDIT5, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%x"),ch1_dcb.XonChar);
	SetDlgItemText(hDlg, IDC_EDIT6, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%x"),ch1_dcb.XoffChar);
	SetDlgItemText(hDlg, IDC_EDIT7, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%x"),ch1_dcb.ErrorChar);
	SetDlgItemText(hDlg, IDC_EDIT8, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%x"),ch1_dcb.EofChar);
	SetDlgItemText(hDlg, IDC_EDIT9, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%x"),ch1_dcb.EvtChar);
	SetDlgItemText(hDlg, IDC_EDIT10, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_dcb.BaudRate);
	SetDlgItemText(hDlg, IDC_EDIT11, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_dcb.ByteSize);
	SetDlgItemText(hDlg, IDC_EDIT12, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_dcb.Parity);
	SetDlgItemText(hDlg, IDC_EDIT13, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_dcb.StopBits);
	SetDlgItemText(hDlg, IDC_EDIT14, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_rstime.ReadIntervalTimeout);
	SetDlgItemText(hDlg, IDC_EDIT15, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_rstime.ReadTotalTimeoutMultiplier);
	SetDlgItemText(hDlg, IDC_EDIT16, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_rstime.ReadTotalTimeoutConstant);
	SetDlgItemText(hDlg, IDC_EDIT17, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_rstime.WriteTotalTimeoutMultiplier);
	SetDlgItemText(hDlg, IDC_EDIT18, s);
	memset(s,NULL,sizeof(s));
	wsprintf(s,TEXT("%d"),ch1_rstime.WriteTotalTimeoutConstant);
	SetDlgItemText(hDlg, IDC_EDIT19, s);
}

//************　設定登録　***************
void Dialog2_out(HWND hDlg)
{
	wchar_t s[32];
	HWND hwndcontrol;

	// ************ CheckBox ***************
	hwndcontrol=GetDlgItem(hDlg, IDC_CHECKBOX1);
	if(SendMessage(hwndcontrol, BM_GETCHECK, 0, 0))
		ch1_dcb.fBinary=1;    //Y
	else
		ch1_dcb.fBinary=0;    //N
	hwndcontrol=GetDlgItem(hDlg, IDC_CHECKBOX2);
	if(SendMessage(hwndcontrol, BM_GETCHECK, 0, 0))
		ch1_dcb.fOutxCtsFlow=1;    //Y
	else
		ch1_dcb.fOutxCtsFlow=0;    //N
	hwndcontrol=GetDlgItem(hDlg, IDC_CHECKBOX3);
	if(SendMessage(hwndcontrol, BM_GETCHECK, 0, 0))
		ch1_dcb.fOutxDsrFlow=1;    //Y
	else
		ch1_dcb.fOutxDsrFlow=0;    //N
	hwndcontrol=GetDlgItem(hDlg, IDC_CHECKBOX4);
	if(SendMessage(hwndcontrol, BM_GETCHECK, 0, 0))
		ch1_dcb.fDsrSensitivity=1;    //Y
	else
		ch1_dcb.fDsrSensitivity=0;    //N
	hwndcontrol=GetDlgItem(hDlg, IDC_CHECKBOX5);
	if(SendMessage(hwndcontrol, BM_GETCHECK, 0, 0))
		ch1_dcb.fTXContinueOnXoff=1;    //Y
	else
		ch1_dcb.fTXContinueOnXoff=0;    //N
	hwndcontrol=GetDlgItem(hDlg, IDC_CHECKBOX6);
	if(SendMessage(hwndcontrol, BM_GETCHECK, 0, 0))
		ch1_dcb.fOutX=1;    //Y
	else
		ch1_dcb.fOutX=0;    //N
	hwndcontrol=GetDlgItem(hDlg, IDC_CHECKBOX7);
	if(SendMessage(hwndcontrol, BM_GETCHECK, 0, 0))
		ch1_dcb.fInX=1;    //Y
	else
		ch1_dcb.fInX=0;    //N
	hwndcontrol=GetDlgItem(hDlg, IDC_CHECKBOX8);
	if(SendMessage(hwndcontrol, BM_GETCHECK, 0, 0))
		ch1_dcb.fErrorChar=1;    //Y
	else
		ch1_dcb.fErrorChar=0;    //N
	hwndcontrol=GetDlgItem(hDlg, IDC_CHECKBOX9);
	if(SendMessage(hwndcontrol, BM_GETCHECK, 0, 0))
		ch1_dcb.fNull=1;    //Y
	else
		ch1_dcb.fNull=0;    //N
	hwndcontrol=GetDlgItem(hDlg, IDC_CHECKBOX10);
	if(SendMessage(hwndcontrol, BM_GETCHECK, 0, 0))
		ch1_dcb.fAbortOnError=1;    //Y
	else
		ch1_dcb.fAbortOnError=0;    //N
	hwndcontrol=GetDlgItem(hDlg, IDC_CHECKBOX11);
	if(SendMessage(hwndcontrol, BM_GETCHECK, 0, 0))
		ch1_dcb.fParity=1;    //Y
	else
		ch1_dcb.fParity=0;    //N

	// ************ editBox ***************
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT1,s,sizeof(s)-2);
	ch1_dcb.fDtrControl=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT2,s,sizeof(s)-2);
	ch1_dcb.fRtsControl=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT3,s,sizeof(s)-2);
	ch1_dcb.fDummy2=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT4,s,sizeof(s)-2);
	ch1_dcb.XonLim=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT5,s,sizeof(s)-2);
	ch1_dcb.XoffLim=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT6,s,sizeof(s)-2);
	ch1_dcb.XonChar=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT7,s,sizeof(s)-2);
	ch1_dcb.XoffChar=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT8,s,sizeof(s)-2);
	ch1_dcb.ErrorChar=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT9,s,sizeof(s)-2);
	ch1_dcb.EofChar=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT10,s,sizeof(s)-2);
	ch1_dcb.EvtChar=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT11,s,sizeof(s)-2);
	ch1_dcb.BaudRate=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT12,s,sizeof(s)-2);
	ch1_dcb.ByteSize=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT13,s,sizeof(s)-2);
	ch1_dcb.Parity=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT14,s,sizeof(s)-2);
	ch1_dcb.StopBits=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT15,s,sizeof(s)-2);
	ch1_rstime.ReadIntervalTimeout=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT16,s,sizeof(s)-2);
	ch1_rstime.ReadTotalTimeoutMultiplier=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT17,s,sizeof(s)-2);
	ch1_rstime.ReadTotalTimeoutConstant=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT18,s,sizeof(s)-2);
	ch1_rstime.WriteTotalTimeoutMultiplier=_wtoi(s);
	memset(s,NULL,sizeof(s));
	GetDlgItemText(hDlg,IDC_EDIT19,s,sizeof(s)-2);
	ch1_rstime.WriteTotalTimeoutConstant=_wtoi(s);

	SetCommState(hCom1, &ch1_dcb);
	SetCommTimeouts(hCom1, &ch1_rstime);

}

//************　設定登録　***************
void Dialog2_reset(HWND hDlg)
{
	ch1_dcb.BaudRate      = 9600;
	ch1_dcb.ByteSize      = 8;
	ch1_dcb.fParity 		 = 0;
	ch1_dcb.Parity        = 0;
	ch1_dcb.StopBits      = 0;
	ch1_dcb.fBinary 		= 1;;
	ch1_dcb.fOutxCtsFlow = 0;
	ch1_dcb.fOutxDsrFlow = 0;
	ch1_dcb.fDsrSensitivity 	= 0;
	ch1_dcb.fTXContinueOnXoff 	= 0;
	ch1_dcb.fOutX 			= 0;
	ch1_dcb.fInX 			= 0;
	ch1_dcb.fErrorChar 	= 0;
	ch1_dcb.fNull 			= 0;
	ch1_dcb.fAbortOnError = 0;
	ch1_dcb.fDtrControl   = 1;
	ch1_dcb.fRtsControl   = 0;
	ch1_dcb.fDummy2       = 0;
	ch1_dcb.XonLim        = 2048;
	ch1_dcb.XoffLim       = 512;
	ch1_dcb.XonChar       = 0x11;
	ch1_dcb.XoffChar      = 0x13;
	ch1_dcb.ErrorChar     = 0;
	ch1_dcb.EofChar       = 0;
	ch1_dcb.EvtChar       = 0x0d;

	ch1_rstime.ReadIntervalTimeout         = 100;
	ch1_rstime.ReadTotalTimeoutMultiplier  = 2;
	ch1_rstime.ReadTotalTimeoutConstant    = 100;
	ch1_rstime.WriteTotalTimeoutMultiplier = 10;
	ch1_rstime.WriteTotalTimeoutConstant   = 1000;

	SetCommState(hCom1, &ch1_dcb);
	SetCommTimeouts(hCom1, &ch1_rstime);
}

// ＤＣＢ＆ＴＩＭＥの設定　Ｄｉａｌｏｇ。
INT_PTR CALLBACK Dialog2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		Dialog2_int(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON1:
			Dialog2_out(hDlg);
		case IDC_BUTTON2:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		case IDC_BUTTON3:
			Dialog2_reset(hDlg);
			Dialog2_int(hDlg);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg,TRUE);
		break;
	case WM_DESTROY:
		break;
	}
	return (INT_PTR)FALSE;
}


//*************************************************************************
//******                 状態表示の処理                            ********
//*************************************************************************

void Dialog3_print_pro(HWND hWnd,HDC hdc,PAINTSTRUCT *ps)
{

	ClearCommError(hCom1,&lpErrorsCH1,&lpStatCH1);
   	lpErrorsCH1_lach|=lpErrorsCH1;

	// ************ エラー情報表示 ******************
	SetBkMode(hdc,TRANSPARENT);
	if(lpErrorsCH1_lach&CE_BREAK)
		TextOut(hdc,10,10,TEXT("●"),1);
	else
		TextOut(hdc,10,10,TEXT("○"),1);

	if(lpErrorsCH1_lach&CE_FRAME)
		TextOut(hdc,10,30,TEXT("●"),1);
	else
		TextOut(hdc,10,30,TEXT("○"),1);

	if(lpErrorsCH1_lach&CE_OVERRUN)
		TextOut(hdc,10,50,TEXT("●"),1);
	else
		TextOut(hdc,10,50,TEXT("○"),1);

	if(lpErrorsCH1_lach&CE_MODE)
		TextOut(hdc,10,70,TEXT("●"),1);
	else
		TextOut(hdc,10,70,TEXT("○"),1);

	if(lpErrorsCH1_lach&CE_IOE)
		TextOut(hdc,10,90,TEXT("●"),1);
	else
		TextOut(hdc,10,90,TEXT("○"),1);

	if(lpErrorsCH1_lach&CE_RXOVER)
		TextOut(hdc,10,110,TEXT("●"),1);
	else
		TextOut(hdc,10,110,TEXT("○"),1);

	if(lpErrorsCH1_lach&CE_RXPARITY)
		TextOut(hdc,10,130,TEXT("●"),1);
	else
		TextOut(hdc,10,130,TEXT("○"),1);

	if(lpErrorsCH1_lach&CE_TXFULL)
		TextOut(hdc,10,150,TEXT("●"),1);
	else
		TextOut(hdc,10,150,TEXT("○"),1);

	// ************ 状態表示 ******************
	if(lpStatCH1.fCtsHold)
		TextOut(hdc,145,10,TEXT("●"),1);
	else
		TextOut(hdc,145,10,TEXT("○"),1);

	if(lpStatCH1.fDsrHold)
		TextOut(hdc,145,30,TEXT("●"),1);
	else
		TextOut(hdc,145,30,TEXT("○"),1);

	if(lpStatCH1.fRlsdHold)
		TextOut(hdc,145,50,TEXT("●"),1);
	else
		TextOut(hdc,145,50,TEXT("○"),1);

	if(lpStatCH1.fXoffHold)
		TextOut(hdc,145,70,TEXT("●"),1);
	else
		TextOut(hdc,145,70,TEXT("○"),1);

	if(lpStatCH1.fXoffSent)
		TextOut(hdc,145,90,TEXT("●"),1);
	else
		TextOut(hdc,145,90,TEXT("○"),1);

	if(lpStatCH1.fEof)
		TextOut(hdc,145,110,TEXT("●"),1);
	else
		TextOut(hdc,145,110,TEXT("○"),1);

	if(lpStatCH1.fTxim)
		TextOut(hdc,145,130,TEXT("●"),1);
	else
		TextOut(hdc,145,130,TEXT("○"),1);

}

// 状態表示のＤｉａｌｏｇ。
INT_PTR CALLBACK Dialog3(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
     	Dialog3_print_pro(hDlg,hdc,&ps);
		EndPaint(hDlg, &ps);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON2:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		case IDC_BUTTON3:
			lpErrorsCH1_lach=0;
			hdc=GetDC(hDlg);
	     	Dialog3_print_pro(hDlg,hdc,&ps);
			ReleaseDC(hDlg,hdc);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg,TRUE);
		break;
	case WM_DESTROY:
		break;
	}
	return (INT_PTR)FALSE;
}

//*************************************************************************
//******                 入力文字の送信                            ********
//*************************************************************************
wc_buf inbuffer[30];

//************　初期化　***************
void Dialog4_int(HWND hDlg)
{
	HWND hwndlist;
	int i;

	hwndlist=GetDlgItem(hDlg, IDC_LIST1);
	SendMessage(hwndlist, LB_RESETCONTENT, 0, 0);
    for(i=0;i<30;i++)
	{
		if(wcslen(inbuffer[i].s)==0)
			break;
		SendMessage(hwndlist,LB_ADDSTRING,0,(LPARAM)inbuffer[i].s);
	}

}

//************　文字を送信　***************
int Dialog4_send(HWND hDlg)
{
	wchar_t s[256];
	char ss[256];
	DWORD send_suu;
	DWORD send_suu2;
	int len;
	int send_err;
	DWORD rs1_err;
	HWND hwndlist;
	int i;

	// ************ editBox ***************
	memset(s,NULL,sizeof(s));
	memset(ss,NULL,sizeof(ss));
	GetDlgItemText(hDlg,IDC_EDIT1,s,sizeof(s)-2);
	
	len=WideCharToMultiByte(CP_THREAD_ACP,WC_NO_BEST_FIT_CHARS,s,-1,NULL,0,NULL,NULL);
	if(len<1)
		return 3;
	WideCharToMultiByte(CP_THREAD_ACP,WC_NO_BEST_FIT_CHARS,s,-1,ss,len,NULL,NULL);

	//send_suu=wcslen(s)*2;
	send_suu=strlen(ss);
	send_suu2=send_suu;

	if(WriteFile(hCom1,ss,send_suu,&rs1_err,&ovlpd_ch1w)==0)
	{
		send_err=GetLastError();
		if(send_err==ERROR_IO_PENDING)
		{
			while(!GetOverlappedResult(hCom1,&ovlpd_ch1w,&send_suu,FALSE));
			if(send_suu!=send_suu2)
			return 2;
		}
		else
		{
			ClearCommError(hCom1,&lpErrorsCH1,&lpStatCH1);
			lpErrorsCH1_lach|=lpErrorsCH1;
			return 1;
		}
	}

    for(i=0;i<30;i++)
	{
		if(memcmp(inbuffer[i].s,s,256)==0)
			return 0;
	}	
	for(i=28;i>-1;i--)
		memcpy(inbuffer[i+1].s,inbuffer[i].s,256);
	memcpy(inbuffer[0].s,s,256);

	hwndlist=GetDlgItem(hDlg, IDC_LIST1);
	SendMessage(hwndlist, LB_RESETCONTENT, 0, 0);
    for(i=0;i<30;i++)
	{
		if(wcslen(inbuffer[i].s)==0)
			break;
		SendMessage(hwndlist,LB_ADDSTRING,0,(LPARAM)inbuffer[i].s);
	}
	return 0;
}

//************　Listboxからの文字取得　***************
void Dialog4_listbox(HWND hDlg)
{
	HWND hwndlist;
	int a;
	
	hwndlist=GetDlgItem(hDlg, IDC_LIST1);
	a=SendMessage(hwndlist,LB_GETCURSEL,0,0);
	SetDlgItemText(hDlg,IDC_EDIT1,inbuffer[a].s);
}

// 状態表示のＤｉａｌｏｇ。
INT_PTR CALLBACK Dialog4(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		Dialog4_int(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_LIST1:
			Dialog4_listbox(hDlg);
			break;
		case IDC_BUTTON1:
			if(Dialog4_send(hDlg))
				MessageBox(hDlg,TEXT("送信出来ませんでした"),TEXT("送信"),MB_OK);
			break;
		case IDC_BUTTON2:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg,TRUE);
		break;
	case WM_DESTROY:
		break;
	}
	return (INT_PTR)FALSE;
}


//*************************************************************************
//******                 設定文字の送信                            ********
//*************************************************************************
BYTE file_dat[256*20];
wc_buf txtbuffer[20];

//************　初期化　***************
void Dialog5_int(HWND hDlg)
{
	HANDLE hFile;
	DWORD f_err;
	int a,b,i;
	BYTE *dat_bas,*dat_poj;
	HWND hwndlist;
	char s[256];
	int len;

	//**** カレントデレクトリの設定 ******
	SetCurrentDirectory(currentdirectory_dt);   //カレントデレクトリの設定



	hFile = CreateFile(TEXT("mdata.txt")                  // ポートの名前
					, GENERIC_READ     	       // Read
					, NULL					          // 共有しない
					, NULL                      // セキュリティ属性デフォルト
					, OPEN_ALWAYS	            // 既存ファイル（無時はエラー）
					, FILE_ATTRIBUTE_NORMAL |   // 属性の指定なし
						FILE_FLAG_SEQUENTIAL_SCAN
					, NULL);
	if (!hFile)
	{
		MessageBox(hDlg,TEXT("ﾌｧｲﾙが開けません"), TEXT("CreateFile"), MB_OK);
		return;
	}

	memset(file_dat,NULL,256*20);
	ReadFile(hFile,&file_dat,256*20,&f_err,NULL);
	CloseHandle(hFile);

	hwndlist=GetDlgItem(hDlg, IDC_LIST1);
	SendMessage(hwndlist, LB_RESETCONTENT, 0, 0);
	a = 0;
	for (i=0;i<20;i++)
	{
		dat_bas = &file_dat[a];
		dat_poj = (BYTE *)memchr(dat_bas,0x0d,256);
		if(dat_poj!=0)
		{
			b = dat_poj-dat_bas;
			memset(s,NULL,sizeof(s));
			memmove(s,dat_bas,b);
			len=MultiByteToWideChar(CP_THREAD_ACP,MB_PRECOMPOSED,s,-1,NULL,NULL);					
			if(len>0)
			{
				MultiByteToWideChar(CP_THREAD_ACP,MB_PRECOMPOSED,s,-1,txtbuffer[i].s,len);					
				SendMessage(hwndlist,LB_ADDSTRING,0,(LPARAM)txtbuffer[i].s);
			}
			a=a+b+2;
		}
	}

}

//************　文字を送信　***************
int Dialog5_send(HWND hDlg)
{
	HWND hwndlist;
	int a;
	char ss[256];
	DWORD send_suu;
	DWORD send_suu2;
	int len;
	int send_err;
	DWORD rs1_err;

	hwndlist=GetDlgItem(hDlg, IDC_LIST1);
	a=SendMessage(hwndlist,LB_GETCURSEL,0,0);

	len=WideCharToMultiByte(CP_THREAD_ACP,WC_NO_BEST_FIT_CHARS,txtbuffer[a].s,-1,NULL,0,NULL,NULL);
	if(len<1)
		return 3;
	WideCharToMultiByte(CP_THREAD_ACP,WC_NO_BEST_FIT_CHARS,txtbuffer[a].s,-1,ss,len,NULL,NULL);

	send_suu=strlen(ss);
	send_suu2=send_suu;

	if(WriteFile(hCom1,ss,send_suu,&rs1_err,&ovlpd_ch1w)==0)
	{
		send_err=GetLastError();
		if(send_err==ERROR_IO_PENDING)
		{
			while(!GetOverlappedResult(hCom1,&ovlpd_ch1w,&send_suu,FALSE));
			if(send_suu!=send_suu2)
			return 2;
		}
		else
		{
			ClearCommError(hCom1,&lpErrorsCH1,&lpStatCH1);
			lpErrorsCH1_lach|=lpErrorsCH1;
			return 1;
		}
	}
	return 0;
}

// 状態表示のＤｉａｌｏｇ。
INT_PTR CALLBACK Dialog5(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		Dialog5_int(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON1:
			if(Dialog5_send(hDlg))
				MessageBox(hDlg,TEXT("送信出来ませんでした"),TEXT("送信"),MB_OK);
			break;
		case IDC_BUTTON2:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg,TRUE);
		break;
	case WM_DESTROY:
		break;
	}
	return (INT_PTR)FALSE;
}
