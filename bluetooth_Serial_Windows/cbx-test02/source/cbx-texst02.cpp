#ifdef __BORLANDC__
  #pragma argsused
#endif

#include <windows.h>
#include "cbx-test02.rc"

// ************** ダイアログの定義 ****************
LRESULT CALLBACK Dlg1Proc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
HINSTANCE g_hInstance;
HINSTANCE hInst;
HWND hWnd2;

/*-------------------------------------------
        変換用 変数
--------------------------------------------*/
char szClassName[] = "cbx-test02";        //ウィンドウクラス

//-----------------------------------------------------------------------------
//       バージョン表示ダイアログ
//-----------------------------------------------------------------------------
LRESULT CALLBACK
Dlg1Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam){

  switch (uMsg){
    case WM_INITDIALOG:
      return TRUE;
    case WM_COMMAND:
      switch (LOWORD( wParam )){
        case IDC_BUTTON1:
          EndDialog( hDlg, TRUE );
          break;
      }
      break;
    case WM_CLOSE:
      EndDialog( hDlg, TRUE );
      break;
  }
  return FALSE;
}


// ========== RS232C & Threadの定義 ==============
// RS232Cの定義
HANDLE hComCH1,hThread;
LPCSTR lpszCommName = "COM1";
//COMMCONFIG cc;
COMMPROP cp;
// オーバーラップ構造体
static OVERLAPPED ovlpd_ch1_read;
static OVERLAPPED ovlpd_ch1_write;
//　Threadの定義
BOOL bReading=FALSE;
DWORD ComReadThread (LPDWORD /*lpdwParam1*/);
DWORD id;
// RS232Cの受信バッファ
char ch1inbuff[1024];
// 表示用バッファ
char plbuff[1024];
int plstrlen=0;


// *************************************************************
// **　　　　　　起動時ＤＣＢとＴＩＭＥの設定　　　　　　        　　 **
// *************************************************************
DCB  ch1_dcb;
COMMTIMEOUTS ch1_rstime;

bool rs_DCBTIM_set1(){
  	ch1_dcb.DCBlength = sizeof(DCB);
   if(SetCommState(hComCH1, &ch1_dcb) == FALSE) {
   	return true;
   }

   if(SetCommTimeouts(hComCH1, &ch1_rstime) == FALSE) {
   	return true;
   }
   return false;
}

void dcb1_data_set(){
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
    ch1_dcb.EvtChar       = 0;
    ch1_rstime.ReadIntervalTimeout         = 100;
    ch1_rstime.ReadTotalTimeoutMultiplier  = 2;
    ch1_rstime.ReadTotalTimeoutConstant    = 100;
    ch1_rstime.WriteTotalTimeoutMultiplier = 10;
    ch1_rstime.WriteTotalTimeoutConstant   = 1000;
}


// ================ com open ===================
int rscom_open(int iPort_no)
{
    char s[8];

    if(iPort_no>0 && iPort_no<256)
    {
   	memset(s,NULL,8);
   	wsprintf(s,"COM%d",iPort_no);
   	lpszCommName = s;
    }
    else
    {
        return 4;                //ポートの割付番号が違っています
    }
    // ComPortを開く
    hComCH1 = CreateFile(lpszCommName                  // ポートの名前
                       ,GENERIC_READ | GENERIC_WRITE  // Read/Write
                       , 0                            // 共有しない
                       , NULL                         // セキュリティ属性デフォルト
                       , OPEN_EXISTING                // 既存ファイル
                       , FILE_FLAG_OVERLAPPED         // 非同期 I/O を許す
                       , NULL);
    if (hComCH1 == INVALID_HANDLE_VALUE)
    {
    	return 1;                   //ポートがオープンしません
    }

    /* get comm properties */
    cp.wPacketLength = sizeof(COMMPROP);
    GetCommProperties (hComCH1, &cp);

    /* Buffer Clear */
    PurgeComm( hComCH1, PURGE_TXABORT | PURGE_RXABORT |
			PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

    if(rs_DCBTIM_set1()==1)
    {
        dcb1_data_set();
   	if(rs_DCBTIM_set1()==1)
        {
            CloseHandle (hComCH1);
            return 2; //ポートの設定はできません
      }
    }
    if (bReading == FALSE){
      // Threadを設定する
      hThread=CreateThread (NULL, //def security
                               0,    //def stack size
               		      (LPTHREAD_START_ROUTINE)ComReadThread,
                            NULL, //param to pass to thread
                               0,
                             &id);
      if (hThread == NULL)
      {
          CloseHandle (hComCH1);
      	  return 3; //スレッドの設定はできません
      }
    }
    ovlpd_ch1_read.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
    bReading = TRUE;
    return 0;             //正常完了
}

// ================ com close ===================
void rscom_close()
{
    bReading = FALSE;
    // イベントを強制的に発行させる
    SetCommMask (hComCH1,EV_RXCHAR);;
    while (true)
    {
        //　Threadが終了したか確認する
        GetExitCodeThread(hThread, &id);
        if (id == STILL_ACTIVE)
            continue;
        else
            break;
    } /* end while (no error reading thread exit code) */
    CloseHandle (hThread);
    PurgeComm(hComCH1,PURGE_RXCLEAR);//input buf クリア
    CloseHandle(hComCH1);
}

// ================ com Read thread ===================
DWORD ComReadThread (LPDWORD /*lpdwParam1*/){
    DWORD dwEvent;
    DWORD dwCH1Read;
    DWORD dwLrc;
    DWORD dwEndtime;
    HDC hDc2;

    SetCommMask (hComCH1,EV_RXCHAR);        //EV_RXCHAR
    while (bReading)
    {
        WaitCommEvent (hComCH1, &dwEvent, NULL);           //I/Oイベント起動
    	if ((dwEvent & EV_RXCHAR) == EV_RXCHAR)
        {
            dwCH1Read=0;
            dwLrc=-1;
            if(ReadFile(hComCH1,ch1inbuff,1024,&dwCH1Read,&ovlpd_ch1_read)==0)
            {
                dwLrc=GetLastError();
		dwCH1Read=0;
		if(dwLrc==ERROR_IO_PENDING)
                {
                    dwEndtime = GetTickCount()+10000;
                    while(!GetOverlappedResult(hComCH1,&ovlpd_ch1_read,&dwCH1Read,FALSE))
                    {
               	        if(GetTickCount() > dwEndtime)
                  	    break;
                    }
                }
            }
            if(dwLrc==0 || dwLrc==ERROR_IO_PENDING)
            {
                if(dwCH1Read)
                {
                    plstrlen=dwCH1Read;
                    memset(plbuff,NULL,1024);
                    memcpy(plbuff,ch1inbuff,plstrlen);
                    InvalidateRect(hWnd2,NULL,FALSE);
                    UpdateWindow(hWnd2);
                }
            }
        }
    }
    return 0;
}

// ================ com Write Program ===================
int send_pro(char *strSendchar,DWORD iSendchar){
    static DWORD dwCH1Write;
    DWORD dwCH1Err;
    DWORD dwLrc;
    int iWriteFileErr;

    dwCH1Write=iSendchar;
    dwLrc=WriteFile(hComCH1,strSendchar,iSendchar,&dwCH1Err,&ovlpd_ch1_write);
    if(dwLrc==0)
    {
        iWriteFileErr=GetLastError();
        if(iWriteFileErr==ERROR_IO_PENDING)
        {
            while(!GetOverlappedResult(hComCH1,&ovlpd_ch1_write,&dwCH1Write,FALSE));
            if(dwCH1Write!=iSendchar)
            {
                return 2;
            }
        }
        else
        {
            return 1;
        }
    }
    return 0;
}

/*-------------------------------------------
        ウィンドウ処理
--------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
  HDC hDC;
  PAINTSTRUCT ps;
  HINSTANCE hInst;
  char s[64];

  switch (msg) {
    case WM_CREATE:
      if(rscom_open(3))
        MessageBox(hWnd, "COMポートが開きません", "cbx-test02", MB_OK);
      hWnd2 = hWnd;
      break;
    case WM_PAINT:
      	hDC = BeginPaint(hWnd, &ps);
        memset(s,0x20,64);
        TextOut(hDC,0,0,s,64);
        TextOut(hDC,0,0,plbuff,plstrlen);
        EndPaint(hWnd, &ps);
      	break;
    case WM_COMMAND:
      switch(LOWORD(wp)) {
        case CM_PEND:  //終了
          if(MessageBox(hWnd, "終了しますか？", "cbx-test02", MB_YESNO) == IDYES)
            SendMessage(hWnd, WM_CLOSE, 0, 0L);
          break;
        case CM_SEND1:
          if(send_pro("1234567890",10))
            MessageBox(hWnd, "送信出来ません", "cbx-test02", MB_OK);
          break;
        case CM_SEND2:
          if(send_pro("ABCDEFGHIJKLMNOPQRSTUVWXYZ",26))
            MessageBox(hWnd, "送信出来ません", "cbx-test02", MB_OK);
          break;
        case CM_SEND3:
          if(send_pro("Ken-Create=RS232C",17))
            MessageBox(hWnd, "送信出来ません", "cbx-test02", MB_OK);
          break;
        case CM_VER:
          DialogBox(g_hInstance,
                    MAKEINTRESOURCE(IDD_DIALOG1),
                    hWnd,(DLGPROC)Dlg1Proc);
          break;
      }
      break;
    case WM_CLOSE:
      DestroyWindow(hWnd);
      break;
    case WM_DESTROY:
      rscom_close();
      PostQuitMessage(0);
      break;
    default:
      return (DefWindowProc(hWnd, msg, wp, lp));
  }
  return 0L;
}

/*-------------------------------------------
        ウィンドウのクラスの処理
--------------------------------------------*/
BOOL InitApp(HINSTANCE hInst)
{
  WNDCLASS wc;
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;    //プロシージャ名
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInst;        //インスタンス
  wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName = MAKEINTRESOURCE(IDM_MENU1);    //メニュー名
  wc.lpszClassName = (LPCSTR)szClassName;
  return (RegisterClass(&wc));
}

/*-------------------------------------------
        ウィンドウの生成
--------------------------------------------*/
BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
  HWND hWnd;
  hWnd = CreateWindow(szClassName,
          "cbx-test02", //タイトルバーにこの名前が表示されます
          WS_OVERLAPPEDWINDOW,    //ウィンドウの種類
          CW_USEDEFAULT,    //Ｘ座標
          CW_USEDEFAULT,    //Ｙ座標
          500,    //幅
          80,    //高さ
          NULL,            //親ウィンドウのハンドル、親を作るときはNULL
          NULL,            //メニューハンドル、クラスメニューを使うときはNULL
          hInst,            //インスタンスハンドル
          NULL);
  if (!hWnd)
    return FALSE;
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);
  return TRUE;
}

/*-------------------------------------------
    　　    メイン関数
--------------------------------------------*/
int WINAPI WinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst,
                   LPSTR lpsCmdLine, int nCmdShow)
{
  MSG msg;
  HWND hFrame, hClient;
  hInst = hCurInst;

  if (!hPrevInst) {
    if (!InitApp(hCurInst))
      return FALSE;
  }
  if (!InitInstance(hCurInst, nCmdShow)) {
    return FALSE;
  }
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (!TranslateMDISysAccel(hClient, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  return msg.wParam;
}
