#include "framework.h"
#include "LoginClient.h"
#include <winsock.h>
#include <process.h>

#pragma comment(lib,"ws2_32.lib")

#define MAX_LOADSTRING 100
#define PORT 3500	//포트번호

struct Info {
    int _order;	//1 로그인 요청, 2 회원가입 요청,  -1 로그인실패, -2 회원가입 실패
    char _name[20];
    char _pwd[20];
};

//소켓프로그래밍 변수
WSADATA wsaData;
SOCKET server_s;
struct sockaddr_in server_addr;
int addr_len;

//winapi
#define ID_EDIT_InputIP     1001
#define ID_EDIT_InputID     1002
#define ID_EDIT_InputPwd     1003
#define ID_EDIT_Result     1004
#define ID_EDIT_ID     1004
#define ID_EDIT_Pwd     1005
#define ID_BUTTON_InputIP   2001
#define ID_BUTTON_Login   2002
#define ID_BUTTON_SignUp   2003

HWND hEdit_inputIP;  
HWND hEdit_inputID;  
HWND hEdit_inputPwd; 
HWND hEdit_Result;   

HANDLE recvThread;  
unsigned WINAPI RecvMsg(void* arg);//쓰레드 수신함수

HINSTANCE hInst;                                
WCHAR szTitle[MAX_LOADSTRING];                  
WCHAR szWindowClass[MAX_LOADSTRING];            

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

unsigned WINAPI RecvMsg(void* arg) {
    SOCKET sock = *((SOCKET*)arg);//서버용 소켓을 전달한다.

    Info* tempInfo;
    char message[1024];
    int strLen;
    while (1) {
        strLen = recv(sock, message, sizeof(message), 0);
        if (strLen <= 0) {
            break;
        }
        message[strLen] = '\0';
        tempInfo = (Info*)message;
        switch (tempInfo->_order)
        {
        case 2:
            SetWindowText(hEdit_Result, L"회원가입 성공");
            break;
        case 1:
            SetWindowText(hEdit_Result, L"로그인 성공");
            break;
        case -2:
            SetWindowText(hEdit_Result, L"회원가입 실패");
            break;
        case -1:
            SetWindowText(hEdit_Result, L"로그인 실패");
            break;
        default:
            break;
        }
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1;
    }
    server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_s == INVALID_SOCKET) {
        return 1;
    }

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LOGINCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LOGINCLIENT));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOGINCLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LOGINCLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      500, 200, 250, 300, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        //edit 생성
        hEdit_inputIP = CreateWindow(L"edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_AUTOVSCROLL, 10, 10, 100, 25, hWnd, (HMENU)ID_EDIT_InputIP, hInst, NULL);
        hEdit_inputID = CreateWindow(L"edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_AUTOVSCROLL, 60, 50, 150, 25, hWnd, (HMENU)ID_EDIT_InputID, hInst, NULL);
        hEdit_inputPwd = CreateWindow(L"edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_AUTOVSCROLL | ES_NUMBER, 60, 90, 150, 25, hWnd, (HMENU)ID_EDIT_InputPwd, hInst, NULL);
        //읽기전용 edit생성
        CreateWindow(L"edit", L"name", WS_CHILD | WS_VISIBLE | WS_BORDER |
        ES_AUTOVSCROLL | ES_READONLY, 10, 50, 50, 25, hWnd, (HMENU)ID_EDIT_ID, hInst, NULL);
        CreateWindow(L"edit", L"pwd", WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_AUTOVSCROLL | ES_READONLY, 10, 90, 50, 25, hWnd, (HMENU)ID_EDIT_Pwd, hInst, NULL);
        hEdit_Result = CreateWindow(L"edit", L"연결대기중", WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_AUTOVSCROLL | ES_READONLY, 10, 170, 200, 25, hWnd, (HMENU)ID_EDIT_Result, hInst, NULL);
        //button 생성
        CreateWindow(L"button", L"접속하기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            110, 10, 100, 25, hWnd, (HMENU)ID_BUTTON_InputIP, hInst, NULL);
        CreateWindow(L"button", L"로그인하기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            110, 130, 100, 25, hWnd, (HMENU)ID_BUTTON_Login, hInst, NULL);
        CreateWindow(L"button", L"회원가입하기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 130, 100, 25, hWnd, (HMENU)ID_BUTTON_SignUp, hInst, NULL);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case ID_BUTTON_InputIP: {
                char inputIP[128];
                wchar_t w_input[128];
                GetWindowText(hEdit_inputIP, w_input, 256);

                int nLen = WideCharToMultiByte(CP_ACP, 0, w_input, -1, NULL, 0, NULL, NULL);
                WideCharToMultiByte(CP_ACP, 0, w_input, -1, inputIP, nLen, NULL, NULL);

                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(PORT);
                server_addr.sin_addr.S_un.S_addr = inet_addr(inputIP); //아이피입력

                if (connect(server_s, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
                    //SetWindowText(hEdit_read, L"연결실패\n"); // hEditOUTPUT의 Text 내보내기
                    closesocket(server_s);  //소켓닫기
                    server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   //소켓 재할당
                }
                else {
                    SetWindowText(hEdit_Result, L"연결성공\r\n"); // hEditOUTPUT의 Text 내보내기
                    recvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&server_s, 0, NULL);//메시지 수신용 쓰레드가 실행된다.
                }

            }
            break;
            case ID_BUTTON_Login: {
                Info tempInfo;
                char c_id[128];
                char c_pwd[128];
                wchar_t w_input[128];

                tempInfo._order = 1;
                GetWindowText(hEdit_inputID, w_input, 256);
                int nLen = WideCharToMultiByte(CP_ACP, 0, w_input, -1, NULL, 0, NULL, NULL);
                WideCharToMultiByte(CP_ACP, 0, w_input, -1, c_id, nLen, NULL, NULL);
                strcpy_s(tempInfo._name, 20, c_id);
                GetWindowText(hEdit_inputPwd, w_input, 256);
                nLen = WideCharToMultiByte(CP_ACP, 0, w_input, -1, NULL, 0, NULL, NULL);
                WideCharToMultiByte(CP_ACP, 0, w_input, -1, c_pwd, nLen, NULL, NULL);
                strcpy_s(tempInfo._pwd, 20 , c_pwd);
                send(server_s, (char*)&tempInfo,sizeof(Info),0);
            }
                break;
            case ID_BUTTON_SignUp: {
                Info tempInfo;
                char c_id[128];
                char c_pwd[128];
                wchar_t w_input[128];

                tempInfo._order = 2;
                GetWindowText(hEdit_inputID, w_input, 256);
                int nLen = WideCharToMultiByte(CP_ACP, 0, w_input, -1, NULL, 0, NULL, NULL);
                WideCharToMultiByte(CP_ACP, 0, w_input, -1, c_id, nLen, NULL, NULL);
                strcpy_s(tempInfo._name, 20, c_id);
                GetWindowText(hEdit_inputPwd, w_input, 256);
                nLen = WideCharToMultiByte(CP_ACP, 0, w_input, -1, NULL, 0, NULL, NULL);
                WideCharToMultiByte(CP_ACP, 0, w_input, -1, c_pwd, nLen, NULL, NULL);
                strcpy_s(tempInfo._pwd, 20, c_pwd);
                send(server_s, (char*)&tempInfo, sizeof(Info), 0);
            }
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
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
