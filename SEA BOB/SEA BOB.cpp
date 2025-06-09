#include "framework.h"
#include "SEA BOB.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "ErrH.h"
#include "FCheck.h"
#include "D2BMPLOADER.h"
#include "gifresizer.h"
#include "SeaDll.h"
#include <chrono>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "fcheck.lib")
#pragma comment(lib, "d2bmploader.lib")
#pragma comment(lib, "gifresizer.lib")
#pragma comment(lib, "seadll.lib")

constexpr wchar_t bWinClassName[]{ L"SEABOB" };

constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };
constexpr wchar_t record_file[]{ L".\\res\\data\\record.dat" };
constexpr wchar_t sound_file[]{ L".\\res\\snd\\main.wav" };

constexpr int mNew{ 1001 };
constexpr int mLvl{ 1002 };
constexpr int mExit{ 1003 };

constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int no_record{ 2001 };
constexpr int first_record{ 2002 };
constexpr int record{ 2003 };

////////////////////////////////////////////////////////////////////

WNDCLASS bWinClass{};
HINSTANCE bIns{ nullptr };
HWND bHwnd{ nullptr };
HICON mainIcon{ nullptr };
HCURSOR mainCur{ nullptr };
HCURSOR outCur{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
PAINTSTRUCT bPaint{};
HDC PaintDC{ nullptr };
MSG bMsg{};
BOOL bRet{ 0 };
UINT bTimer{ 0 };

POINT cur_pos{};

D2D1_RECT_F b1Rect{ 0, 0, scr_width / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_width / 3, 0, scr_width * 2 / 3 - 50.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_width * 2 / 3, 0, scr_width - 50.0f, 50.0f };

D2D1_RECT_F b1TxtRect{ 50.0f, 10.0f, scr_width / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2TxtRect{ scr_width / 3 + 50.0f, 10.0f, scr_width * 2 / 3 - 50.0f, 50.0f };
D2D1_RECT_F b3TxtRect{ scr_width * 2 / 3 + 50.0f, 10.0f, scr_width - 50.0f, 50.0f };

wchar_t current_player[16]{ L"BOBBY" };

bool pause = false;
bool in_client = true;
bool show_help = false;
bool sound = true;
bool name_set = false;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;

bool level_skipped = false;

int level = 1;
int score = 0;

int mins = 0;
int secs = 180;

int jellies_saved = 0;
int jellies_lost = 0;
int need_to_save = 0;

//////////////////////////////////////////////////

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };

ID2D1RadialGradientBrush* but1BckgBrush{ nullptr };
ID2D1RadialGradientBrush* but2BckgBrush{ nullptr };
ID2D1RadialGradientBrush* but3BckgBrush{ nullptr };

ID2D1SolidColorBrush* statBckgBrush{ nullptr };
ID2D1SolidColorBrush* txtBrush{ nullptr };
ID2D1SolidColorBrush* hgltBrush{ nullptr };
ID2D1SolidColorBrush* inactBrush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* nrmFormat{ nullptr };
IDWriteTextFormat* midFormat{ nullptr };
IDWriteTextFormat* bigFormat{ nullptr };

ID2D1Bitmap* bmpIntro[24]{ nullptr };
ID2D1Bitmap* bmpField[75]{ nullptr };

ID2D1Bitmap* bmpBubbles[40]{ nullptr };
ID2D1Bitmap* bmpGrass[6]{ nullptr };

ID2D1Bitmap* bmpBob[2]{ nullptr };
ID2D1Bitmap* bmpJelly[4]{ nullptr };

ID2D1Bitmap* bmpEvil1L[49]{ nullptr };
ID2D1Bitmap* bmpEvil1R[49]{ nullptr };

ID2D1Bitmap* bmpEvil2L[12]{ nullptr };
ID2D1Bitmap* bmpEvil2R[12]{ nullptr };

ID2D1Bitmap* bmpEvil3L[6]{ nullptr };
ID2D1Bitmap* bmpEvil3R[6]{ nullptr };

ID2D1Bitmap* bmpEvil4L[20]{ nullptr };
ID2D1Bitmap* bmpEvil4R[20]{ nullptr };

ID2D1Bitmap* bmpEvil5L[8]{ nullptr };
ID2D1Bitmap* bmpEvil5R[8]{ nullptr };

ID2D1Bitmap* bmpEvil6L[40]{ nullptr };
ID2D1Bitmap* bmpEvil6R[40]{ nullptr };

ID2D1Bitmap* bmpEvil7L[12]{ nullptr };
ID2D1Bitmap* bmpEvil7R[12]{ nullptr };

ID2D1Bitmap* bmpEvil8L[16]{ nullptr };
ID2D1Bitmap* bmpEvil8R[16]{ nullptr };

////////////////////////////////////////////////////////////

dll::RANDIt Randerer{};

dll::GameObject Field{ nullptr };
std::vector<dll::GameObject> vSeaGrass;
std::vector<dll::GameObject> vBubbles;

dll::GameCreature Bob{ nullptr };
std::vector<dll::GameCreature>vJellies;
std::vector<dll::GameCreature>vEvils;

///////////////////////////////////////////////////////////

template<typename T>concept HasRelease = requires (T check)
{
    check.Release();
};
template<HasRelease T> bool ClearHeap(T** var)
{
    if ((*var))
    {
        (*var)->Release();
        (*var) = nullptr;
        return true;
    }
    return false;
};
void LogErr(LPCWSTR what)
{
    std::wofstream err(L".\\res\\data\\error.log", std::ios::app);
    err << what << L" Time stamp: " << std::chrono::system_clock::now() << std::endl;
    err.close();
}
void ReleaseResources()
{
    if (!ClearHeap(&iFactory))LogErr(L" Error releasing iFactory !");
    if (!ClearHeap(&Draw))LogErr(L" Error releasing Draw !");
    if (!ClearHeap(&but1BckgBrush))LogErr(L" Error releasing but1BckgBrush !");
    if (!ClearHeap(&but2BckgBrush))LogErr(L" Error releasing but2BckgBrush !");
    if (!ClearHeap(&but3BckgBrush))LogErr(L" Error releasing but3BckgBrush !");
    if (!ClearHeap(&statBckgBrush))LogErr(L" Error releasing statBckgBrush !");
    if (!ClearHeap(&txtBrush))LogErr(L" Error releasing txtBrush !");
    if (!ClearHeap(&hgltBrush))LogErr(L" Error releasing hgltBrush !");
    if (!ClearHeap(&inactBrush))LogErr(L" Error releasing inactBrush !");
    if (!ClearHeap(&iWriteFactory))LogErr(L" Error releasing iWriteFactory !");
    if (!ClearHeap(&nrmFormat))LogErr(L" Error releasing nrmFormat !");
    if (!ClearHeap(&midFormat))LogErr(L" Error releasing midFormat !");
    if (!ClearHeap(&bigFormat))LogErr(L" Error releasing bigFormat !");

    for (int i = 0; i < 24; ++i)if (!ClearHeap(&bmpIntro[i]))LogErr(L" Error releasing bmpIntro !");
    for (int i = 0; i < 75; ++i)if (!ClearHeap(&bmpField[i]))LogErr(L" Error releasing bmpField !");
    for (int i = 0; i < 40; ++i)if (!ClearHeap(&bmpBubbles[i]))LogErr(L" Error releasing bmpBubbles !");
    for (int i = 0; i < 6; ++i)if (!ClearHeap(&bmpGrass[i]))LogErr(L" Error releasing bmpGrass !");

    
    for (int i = 0; i < 2; ++i)if (!ClearHeap(&bmpBob[i]))LogErr(L" Error releasing bmpBob !");
    for (int i = 0; i < 4; ++i)if (!ClearHeap(&bmpJelly[i]))LogErr(L" Error releasing bmpJelly !");

    for (int i = 0; i < 49; ++i)if (!ClearHeap(&bmpEvil1L[i]))LogErr(L" Error releasing bmpEvil1L !");
    for (int i = 0; i < 49; ++i)if (!ClearHeap(&bmpEvil1R[i]))LogErr(L" Error releasing bmpEvil1R !");

    for (int i = 0; i < 12; ++i)if (!ClearHeap(&bmpEvil2L[i]))LogErr(L" Error releasing bmpEvil2L !");
    for (int i = 0; i < 12; ++i)if (!ClearHeap(&bmpEvil2R[i]))LogErr(L" Error releasing bmpEvil2R !");

    for (int i = 0; i < 6; ++i)if (!ClearHeap(&bmpEvil3L[i]))LogErr(L" Error releasing bmpEvil3L !");
    for (int i = 0; i < 6; ++i)if (!ClearHeap(&bmpEvil3R[i]))LogErr(L" Error releasing bmpEvil3R !");

    for (int i = 0; i < 20; ++i)if (!ClearHeap(&bmpEvil4L[i]))LogErr(L" Error releasing bmpEvil4L !");
    for (int i = 0; i < 20; ++i)if (!ClearHeap(&bmpEvil4R[i]))LogErr(L" Error releasing bmpEvil4R !");

    for (int i = 0; i < 8; ++i)if (!ClearHeap(&bmpEvil5L[i]))LogErr(L" Error releasing bmpEvil5L !");
    for (int i = 0; i < 8; ++i)if (!ClearHeap(&bmpEvil5R[i]))LogErr(L" Error releasing bmpEvil5R !");

    for (int i = 0; i < 40; ++i)if (!ClearHeap(&bmpEvil6L[i]))LogErr(L" Error releasing bmpEvil6L !");
    for (int i = 0; i < 40; ++i)if (!ClearHeap(&bmpEvil6R[i]))LogErr(L" Error releasing bmpEvil6R !");

    for (int i = 0; i < 12; ++i)if (!ClearHeap(&bmpEvil7L[i]))LogErr(L" Error releasing bmpEvil7L !");
    for (int i = 0; i < 12; ++i)if (!ClearHeap(&bmpEvil7R[i]))LogErr(L" Error releasing bmpEvil7R !");

    for (int i = 0; i < 16; ++i)if (!ClearHeap(&bmpEvil8L[i]))LogErr(L" Error releasing bmpEvil8L !");
    for (int i = 0; i < 16; ++i)if (!ClearHeap(&bmpEvil8R[i]))LogErr(L" Error releasing bmpEvil8R !");
}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"Критична грешка !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    std::remove(tmp_file);
    exit(1);
}

void GameOver()
{
    PlaySound(NULL, NULL, NULL);
    KillTimer(bHwnd, bTimer);


    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void InitGame()
{
    wcscpy_s(current_player, L"BOBBY");
    name_set = false;
    score = 0;
    level = 1;
    level_skipped = false;

    ////////////////////////////////////

    jellies_saved = 0;
    jellies_lost = 0;
    need_to_save = 10 + level;

    mins = 0;
    secs = 180 - level * 5;

    if (Field)Field->ObjRelease();
    Field = dll::CreateObject(types::field, 0, 50.0f);

    if (!vBubbles.empty())for (int i = 0; i < vBubbles.size(); ++i) vBubbles[i]->ObjRelease();
    vBubbles.clear();

    if (!vSeaGrass.empty())for (int i = 0; i < vSeaGrass.size(); ++i) vSeaGrass[i]->ObjRelease();
    vSeaGrass.clear();

    vSeaGrass.push_back(dll::CreateObject(types::grass, (float)(Randerer(10, 80)), ground - 157.0f));
    if (!vSeaGrass.empty())
    {
        for (int i = 0; i < 4; ++i)
            vSeaGrass.push_back(dll::CreateObject(types::grass, vSeaGrass.back()->end.x + (float)(Randerer(100, 150)),
                ground - 157.0f));
    }

    if (Bob)Bob->Release();
    Bob = dll::CreateCreature(types::hero, (float)(Randerer(10, (int)(scr_width - 80.0f))), ground - 80.0f, 0, 0);

    if (!vJellies.empty())for (int i = 0; i < vJellies.size(); ++i) vJellies[i]->Release();
    vJellies.clear();

    if (!vEvils.empty())for (int i = 0; i < vEvils.size(); ++i) vEvils[i]->Release();
    vEvils.clear();
}
void LevelUp()
{
    PlaySound(NULL, NULL, NULL);

    if (!level_skipped)
    {
        int bonus = secs / 5 + jellies_saved - jellies_lost;

        if (bonus > 0)
        {

        }
        
    }

    if (sound)
    {
        PlaySound(L".\\res\\snd\\levelup", NULL, SND_SYNC);
        PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
    }
    else Sleep(3000);



    level_skipped = false;
    ++level;

    jellies_saved = 0;
    jellies_lost = 0;
    need_to_save = 10 + level;

    mins = 0;
    secs = 180 - level * 5;

    if (Field)Field->ObjRelease();
    Field = dll::CreateObject(types::field, 0, 50.0f);

    if (!vBubbles.empty())for (int i = 0; i < vBubbles.size(); ++i) vBubbles[i]->ObjRelease();
    vBubbles.clear();

    if (!vSeaGrass.empty())for (int i = 0; i < vSeaGrass.size(); ++i) vSeaGrass[i]->ObjRelease();
    vSeaGrass.clear();

    vSeaGrass.push_back(dll::CreateObject(types::grass, (float)(Randerer(10, 80)), ground - 157.0f));
    if (!vSeaGrass.empty())
    {
        for (int i = 0; i < 4; ++i)
            vSeaGrass.push_back(dll::CreateObject(types::grass, vSeaGrass.back()->end.x + (float)(Randerer(100, 150)),
                ground - 157.0f));
    }

    if (Bob)Bob->Release();
    Bob = dll::CreateCreature(types::hero, (float)(Randerer(10, (int)(scr_width - 80.0f))), ground - 80.0f, 0, 0);

    if (!vJellies.empty())for (int i = 0; i < vJellies.size(); ++i) vJellies[i]->Release();
    vJellies.clear();

    if (!vEvils.empty())for (int i = 0; i < vEvils.size(); ++i) vEvils[i]->Release();
    vEvils.clear();
}


INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(mainIcon));
        return true;

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDOK:
            if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
                MessageBox(bHwnd, L"Името си ли забрави ?", L"Забраватор", MB_OK | MB_APPLMODAL | MB_ICONQUESTION);
                EndDialog(hwnd, IDCANCEL);
                break;
            }
            EndDialog(hwnd, IDOK);
            break;
        }
        break;
    }

    return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
        if (bIns)
        {
            SetTimer(hwnd, bTimer, 1000, NULL);
            bBar = CreateMenu();
            bMain = CreateMenu();
            bStore = CreateMenu();

            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

            AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
            AppendMenu(bMain, MF_STRING, mLvl, L"Следващо ниво");
            AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bMain, MF_STRING, mExit, L"Изход");

            AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
            AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
            AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");

            SetMenu(hwnd, bBar);
            InitGame();
        }
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(hwnd, L"Ако излезеш, губиш прогреса по тази игра !\n\nНаистина ли излизаш ?", L"Изход !",
            MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(0, 0, 0)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cur_pos);
        ScreenToClient(hwnd, &cur_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                in_client = true;
                pause = false;
            }

            if (cur_pos.y <= 50)
            {
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = true;
                        b2Hglt = false;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = true;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = false;
                        b3Hglt = true;
                    }
                }

                SetCursor(outCur);
                return true;
            }
            else if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }

            SetCursor(mainCur);
            return true;
        }
        else
        {
            if (in_client)
            {
                in_client = false;
                pause = true;
            }

            if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }
        
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return true;
        }
        break;

    case WM_TIMER:
        if (pause)break;
        --secs;
        mins = secs / 60;
        if (secs <= 0)LevelUp();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case mNew:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"При рестарт губиш прогреса по тази игра !\n\nНаистина ли рестартираш ?", L"Рестарт !",
                MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            break;

        case mLvl:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако прескочиш ниво, губиш прогреса по него !\n\nНаистина ли прескачаш нивото ?", L"Следващо ниво !",
                MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO) == IDNO)
            {
                pause = false;
                break;
            }
            level_skipped = true;
            LevelUp();
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;


        }
        break;

    case WM_LBUTTONDOWN:
        if (HIWORD(lParam) <= 50)
        {
            if (LOWORD(lParam) >= b1Rect.left && LOWORD(lParam) <= b1Rect.right)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &DlgProc) == IDOK)name_set = true;
                break;
            }
            if (LOWORD(lParam) >= b2Rect.left && LOWORD(lParam) <= b2Rect.right)
            {
                mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                
                if (sound)
                {
                    sound = false;
                    PlaySound(NULL, NULL, NULL);
                    break;
                }
                else
                {
                    PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
                    break;
                }
            }
        }
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_LEFT:
            Bob->dir = dirs::left;
            Bob->Move((float)(level));
            break;

        case VK_RIGHT:
            Bob->dir = dirs::right;
            Bob->Move((float)(level));
            break;

        case VK_UP:
            Bob->dir = dirs::up;
            Bob->Move((float)(level));
            break;

        case VK_DOWN:
            Bob->dir = dirs::down;
            Bob->Move((float)(level));
            break;
        }
        break;

    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void CreateResources()
{
    int winx = (int)(GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2));
    int winy = 10;

    if (GetSystemMetrics(SM_CXSCREEN) < (int)(scr_width) + winx || GetSystemMetrics(SM_CYSCREEN) < (int)(scr_height) + winy)
        ErrExit(eScreen);

    mainIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 255, LR_LOADFROMFILE));
    if (!mainIcon)ErrExit(eIcon);

    mainCur = LoadCursorFromFileW(L".\\res\\main.ani");
    outCur = LoadCursorFromFileW(L".\\res\\out.ani");

    if (!mainCur || !outCur)ErrExit(eCursor);

    ZeroMemory(&bWinClass, sizeof(WNDCLASS));

    bWinClass.lpszClassName = bWinClassName;
    bWinClass.hInstance = bIns;
    bWinClass.lpfnWndProc = &WinProc;
    bWinClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    bWinClass.hCursor = mainCur;
    bWinClass.hIcon = mainIcon;
    bWinClass.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWinClass))ErrExit(eClass);

    bHwnd = CreateWindow(bWinClassName, L"СПОНДЖ БОБ - СПАСИТЕЛ !", WS_CAPTION | WS_SYSMENU, winx, winy, (int)(scr_width),
        (int)(scr_height), NULL, NULL, bIns, NULL);

    if (!bHwnd)ErrExit(eWindow);
    else
    {
        ShowWindow(bHwnd, SW_SHOWDEFAULT);

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
        if (hr != S_OK)
        {
            LogErr(L"Error creating iFactory !");
            ErrExit(eD2D);
        }
        if (iFactory)
            hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
                D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
        if (hr != S_OK)
        {
            LogErr(L"Error creating Draw !");
            ErrExit(eD2D);
        }

        if (Draw)
        {
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BurlyWood), &statBckgBrush);
            if (hr != S_OK)
            {
                LogErr(L"Error creating statBckgBrush !");
                ErrExit(eD2D);
            }

            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Lime), &txtBrush);
            if (hr != S_OK)
            {
                LogErr(L"Error creating txtBrush !");
                ErrExit(eD2D);
            }

            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGoldenrod), &hgltBrush);
            if (hr != S_OK)
            {
                LogErr(L"Error creating hgltBrush !");
                ErrExit(eD2D);
            }

            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BlueViolet), &inactBrush);
            if (hr != S_OK)
            {
                LogErr(L"Error creating inactBrush !");
                ErrExit(eD2D);
            }

            D2D1_GRADIENT_STOP gStops[2]{};
            ID2D1GradientStopCollection* gColl{ nullptr };

            gStops[0].position = 0;
            gStops[0].color = D2D1::ColorF(D2D1::ColorF::MediumTurquoise);
            gStops[1].position = 1.0f;
            gStops[1].color = D2D1::ColorF(D2D1::ColorF::Maroon);

            hr = Draw->CreateGradientStopCollection(gStops, 2, &gColl);
            if (hr != S_OK)
            {
                LogErr(L"Error creating gColl !");
                ErrExit(eD2D);
            }

            if (gColl)
            {
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b1Rect.left + 
                    (b1Rect.right - b1Rect.left) / 2, 25.0f), D2D1::Point2F(0, 0), (b1Rect.right - b1Rect.left) / 2, 25.0f), 
                    gColl, &but1BckgBrush);

                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b2Rect.left +
                    (b2Rect.right - b2Rect.left) / 2, 25.0f), D2D1::Point2F(0, 0), (b2Rect.right - b2Rect.left) / 2, 25.0f),
                    gColl, &but2BckgBrush);

                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b3Rect.left +
                    (b3Rect.right - b3Rect.left) / 2, 25.0f), D2D1::Point2F(0, 0), (b3Rect.right - b3Rect.left) / 2, 25.0f),
                    gColl, &but3BckgBrush);
            
                if (hr != S_OK)
                {
                    LogErr(L"Error creating butBckgBrushes !");
                    ErrExit(eD2D);
                }

                ClearHeap(&gColl);
            }

            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\intro\\0";
                wchar_t  add[8] = L"\0";

                if (i < 10)wcscat_s(name, L"0");
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpIntro[i] = Load(name, Draw);
                if (!bmpIntro[i])
                {
                    LogErr(L"Error loading bmpIntro !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 75; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\field\\0";
                wchar_t  add[8] = L"\0";

                if (i < 10)wcscat_s(name, L"0");
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpField[i] = Load(name, Draw);
                if (!bmpField[i])
                {
                    LogErr(L"Error loading bmpField !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 40; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\bubbles\\";
                wchar_t add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpBubbles[i] = Load(name, Draw);
                if (!bmpBubbles[i])
                {
                    LogErr(L"Error loading bmpBubbles !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 6; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\grass\\";
                wchar_t add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpGrass[i] = Load(name, Draw);
                if (!bmpGrass[i])
                {
                    LogErr(L"Error loading bmpGrass !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 2; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\bob\\";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpBob[i] = Load(name, Draw);
                if (!bmpBob[i])
                {
                    LogErr(L"Error loading bmpBob !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 4; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\jellyfish\\";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpJelly[i] = Load(name, Draw);
                if (!bmpJelly[i])
                {
                    LogErr(L"Error loading bmpJelly !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 49; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\1\\l\\0";
                wchar_t  add[8] = L"\0";

                if (i < 10)wcscat_s(name, L"0");
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil1L[i] = Load(name, Draw);
                if (!bmpEvil1L[i])
                {
                    LogErr(L"Error loading bmpEvil1L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 49; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\1\\r\\0";
                wchar_t  add[8] = L"\0";

                if (i < 10)wcscat_s(name, L"0");
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil1R[i] = Load(name, Draw);
                if (!bmpEvil1R[i])
                {
                    LogErr(L"Error loading bmpEvil1R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 12; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\2\\l\\0";
                wchar_t  add[8] = L"\0";

                if (i < 10)wcscat_s(name, L"0");
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil2L[i] = Load(name, Draw);
                if (!bmpEvil2L[i])
                {
                    LogErr(L"Error loading bmpEvil2L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 12; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\2\\r\\0";
                wchar_t  add[8] = L"\0";

                if (i < 10)wcscat_s(name, L"0");
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil2R[i] = Load(name, Draw);
                if (!bmpEvil2R[i])
                {
                    LogErr(L"Error loading bmpEvil2R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 6; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\3\\l\\0";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil3L[i] = Load(name, Draw);
                if (!bmpEvil3L[i])
                {
                    LogErr(L"Error loading bmpEvil3L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 6; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\3\\r\\0";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil3R[i] = Load(name, Draw);
                if (!bmpEvil3R[i])
                {
                    LogErr(L"Error loading bmpEvil3R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 20; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\4\\l\\0";
                wchar_t  add[8] = L"\0";

                if (i < 10)wcscat_s(name, L"0");
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil4L[i] = Load(name, Draw);
                if (!bmpEvil4L[i])
                {
                    LogErr(L"Error loading bmpEvil4L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 20; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\4\\r\\0";
                wchar_t  add[8] = L"\0";

                if (i < 10)wcscat_s(name, L"0");
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil4R[i] = Load(name, Draw);
                if (!bmpEvil4R[i])
                {
                    LogErr(L"Error loading bmpEvil4R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 8; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\5\\l\\";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil5L[i] = Load(name, Draw);
                if (!bmpEvil5L[i])
                {
                    LogErr(L"Error loading bmpEvil5L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 8; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\5\\r\\";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil5R[i] = Load(name, Draw);
                if (!bmpEvil5R[i])
                {
                    LogErr(L"Error loading bmpEvil5R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 40; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\6\\l\\";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil6L[i] = Load(name, Draw);
                if (!bmpEvil6L[i])
                {
                    LogErr(L"Error loading bmpEvil6L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 40; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\6\\r\\";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil6R[i] = Load(name, Draw);
                if (!bmpEvil6R[i])
                {
                    LogErr(L"Error loading bmpEvil6R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 12; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\7\\l\\";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil7L[i] = Load(name, Draw);
                if (!bmpEvil7L[i])
                {
                    LogErr(L"Error loading bmpEvil7L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 12; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\7\\r\\";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil7R[i] = Load(name, Draw);
                if (!bmpEvil7R[i])
                {
                    LogErr(L"Error loading bmpEvil7R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 16; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\8\\l\\";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil8L[i] = Load(name, Draw);
                if (!bmpEvil8L[i])
                {
                    LogErr(L"Error loading bmpEvil8L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 16; ++i)
            {
                wchar_t name[160] = L".\\res\\img\\evils\\8\\r\\";
                wchar_t  add[8] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil8R[i] = Load(name, Draw);
                if (!bmpEvil8R[i])
                {
                    LogErr(L"Error loading bmpEvil8R !");
                    ErrExit(eD2D);
                }
            }
        }

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&iWriteFactory));
        if (hr != S_OK)
        {
            LogErr(L"Error creating iWriteFactory !");
            ErrExit(eD2D);
        }

        if (iWriteFactory)
        {
            hr = iWriteFactory->CreateTextFormat(L"COMIC SANS", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 16, L"", &nrmFormat);

            hr = iWriteFactory->CreateTextFormat(L"COMIC SANS", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 24, L"", &midFormat);

            hr = iWriteFactory->CreateTextFormat(L"COMIC SANS", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 64, L"", &bigFormat);

            if (hr != S_OK)
            {
                LogErr(L"Error creating text formats !");
                ErrExit(eD2D);
            }
        }
    }

    wchar_t intro_text[46]{ L"СПОНДЖ БОБ - СПАСИТЕЛ !\n\n       dev. Daniel !" };
    wchar_t show_txt[46] = L"\0";
    int txt_marker = 0;
    
    int intro_frame = 0;

    while (txt_marker < 46)
    {
        if (Draw && bigFormat && txtBrush)
        {
            Draw->BeginDraw();
            Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
            ++intro_frame;
            if (intro_frame > 23)intro_frame = 0;
            show_txt[txt_marker] = intro_text[txt_marker];
            if (show_txt[txt_marker] != L' ' && show_txt[txt_marker] != L'\n')
                mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
            ++txt_marker;
            Draw->DrawTextW(show_txt, txt_marker, bigFormat, D2D1::RectF(10.0f, 200.0f, scr_width, scr_height), txtBrush);
            Draw->EndDraw();
            Sleep(40);
        }
    }    
    PlaySound(L".\\res\\snd\\intro.wav", NULL, SND_SYNC);
}

/////////////////////////////////////////////////////////////

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)
    {
        LogErr(L"Error obtaining hInstance from Windows !");
        ErrExit(eClass);
    }

    CreateResources();

    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);

            TranslateMessage(&bMsg);
            DispatchMessage(&bMsg);
        }

        if (pause)
        {
            if (show_help)continue;
            if (bigFormat && txtBrush)
            {
                Draw->BeginDraw();
                static int int_frame = 0;
                Draw->DrawBitmap(bmpIntro[int_frame], D2D1::RectF(0, 0, scr_width, scr_height));
                ++int_frame;
                if (int_frame > 23)int_frame = 0;
                Draw->DrawTextW(L"ПАУЗА", 6, bigFormat, D2D1::RectF(scr_width / 2 - 100.0f, scr_height / 2 - 50.0f, scr_width, scr_height),
                    txtBrush);
                Draw->EndDraw();
                continue;
            }
        }

        /////////////////////////////////////////////

        if (vJellies.size() <= level + 3 && Randerer(0, 200) == 66)
        {
            float sx = (float)(Randerer(20, (int)(scr_width)));
            float sy = 0;
            float ex = 0;
            float ey = 0;

            switch (Randerer(0, 1))
            {
            case 0:
                sy = sky;
                break;

            case 1:
                sy = ground;
                break;
            }

            if (sx < scr_width / 2)ex = scr_width - (float)(Randerer(0, 300));
            else if (sx > scr_width / 2)ex = (float)(Randerer(0, 300));
            else ex = sx;

            if (sy < scr_height / 2)ey = ground;
            else if (sy > scr_height / 2)ey = sky;
            else ey = sy;

            vJellies.push_back(dll::CreateCreature(types::jelly, sx, sy, ex, ey));
        }
        
        if (!vJellies.empty())
        {
            for (std::vector<dll::GameCreature>::iterator jel = vJellies.begin(); jel < vJellies.end(); ++jel)
            {
                if (!(*jel)->Move((float)(level)))
                {
                    (*jel)->Release();
                    vJellies.erase(jel);
                    break;
                }
            }
        }

        /////////////////////////////////////////////

        if (vEvils.size() <= level + 3 && Randerer(0, 300) == 66)
        {
            float sx = (float)(Randerer(20, (int)(scr_width)));
            float sy = (float)(Randerer((int)(sky), (int)(ground-100.0f)));
            float ex = 0;
            float ey = 0;

            if (sx < scr_width / 2)ex = scr_width - (float)(Randerer(0, 300));
            else if (sx > scr_width / 2)ex = (float)(Randerer(0, 300));
            else ex = sx;

            if (sy < scr_height / 2)ey = ground - 100.0f;
            else if (sy > scr_height / 2)ey = sky;
            else ey = sy;

            vEvils.push_back(dll::CreateCreature(static_cast<types>(Randerer(0, 7)), sx, sy, ex, ey));
        }

        if (!vEvils.empty() && Bob)
        {
            dll::PACK<FPOINT> Mesh(1 + vJellies.size());
            Mesh.push_back(Bob->center);
            
            if (!vJellies.empty())
               for (int i = 0; i < vJellies.size(); ++i)Mesh.push_back(vJellies[i]->center);

            for (std::vector<dll::GameCreature>::iterator ev = vEvils.begin(); ev < vEvils.end(); ++ev)
                (*ev)->EvilAI(Mesh, (float)(level));
        }

        if (vBubbles.size() < 5 && Randerer(0, 300) == 66)
        {
            vBubbles.push_back(dll::CreateObject(types::bubbles, (float)(Randerer(20, 900)), ground - 80.0f));
            if (sound)mciSendString(L"play .\\res\\snd\\bubbles.wav", NULL, NULL, NULL);
        }
        if (!vBubbles.empty())
        {
            for (std::vector<dll::GameObject>::iterator bub = vBubbles.begin(); bub < vBubbles.end(); ++bub)
            {
                (*bub)->start.y--;
                (*bub)->SetEdges();
                if ((*bub)->start.y <= sky)
                {
                    (*bub)->ObjRelease();
                    vBubbles.erase(bub);
                    break;
                }
            }
        }

        //////////////////////////////////////////////

        if (Bob && !vJellies.empty())
        {
            for (std::vector<dll::GameCreature>::iterator jel = vJellies.begin(); jel < vJellies.end(); ++jel)
            {
                if ((abs(Bob->center.x - (*jel)->center.x) <= Bob->XRadius() + (*jel)->XRadius())
                    && (abs(Bob->center.y - (*jel)->center.y) <= Bob->YRadius() + (*jel)->YRadius()))
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\savedjelly.wav", NULL, NULL, NULL);
                    jellies_saved++;
                    score += 4 + level;

                    (*jel)->Release();
                    vJellies.erase(jel);
                    break;
                }
            }
        }
        if (!vEvils.empty() && !vJellies.empty())
        {
            for (std::vector<dll::GameCreature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                bool ate = false;

                for (std::vector<dll::GameCreature>::iterator jel = vJellies.begin(); jel < vJellies.end(); ++jel)
                {
                    if ((abs((*evil)->center.x - (*jel)->center.x) <= (*evil)->XRadius() + (*jel)->XRadius())
                        && (abs((*evil)->center.y - (*jel)->center.y) <= (*evil)->YRadius() + (*jel)->YRadius()))
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\eaten.wav", NULL, NULL, NULL);
                        jellies_lost++;
                        ate = true;
                        (*jel)->Release();
                        vJellies.erase(jel);
                        break;
                    }
                }

                if (ate) break;
            }

        }
        if (Bob && !vEvils.empty())
        {
            for (std::vector<dll::GameCreature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                if ((abs(Bob->center.x - (*evil)->center.x) <= Bob->XRadius() + (*evil)->XRadius())
                    && (abs(Bob->center.y - (*evil)->center.y) <= Bob->YRadius() + (*evil)->YRadius()))
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\defeat.wav", NULL, NULL, NULL);
                    Bob->Release();
                    Bob = nullptr;
                    break;
                }
            }
            if (!Bob)
            {
                Sleep(300);
                GameOver();
            }
        }

        if (jellies_saved >= need_to_save)LevelUp();
        if (jellies_lost >= 15 + level)GameOver();

        // DRAW THINGS ******************************

        Draw->BeginDraw();

        if (statBckgBrush && but1BckgBrush && but2BckgBrush && but3BckgBrush && txtBrush && hgltBrush && txtBrush && nrmFormat)
        {
            Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), statBckgBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b1Rect, (b1Rect.right - b1Rect.left) / 2, 25.0f), but1BckgBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b2Rect, (b2Rect.right - b2Rect.left) / 2, 25.0f), but2BckgBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b3Rect, (b3Rect.right - b3Rect.left) / 2, 25.0f), but3BckgBrush);

            if (name_set)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmFormat, b1TxtRect, inactBrush);
            else
            {
                if (b1Hglt)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmFormat, b1TxtRect, hgltBrush);
                else Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmFormat, b1TxtRect, txtBrush);
            }
            if (b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2TxtRect, hgltBrush);
            else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2TxtRect, txtBrush);
            if (b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, hgltBrush);
            else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, txtBrush);
        }

        Draw->DrawBitmap(bmpField[Field->GetFrame()], D2D1::RectF(Field->start.x, Field->start.y, Field->end.x, Field->end.y));

        if (Bob)
        {
            int frame = Bob->GetFrame();
            Draw->DrawBitmap(bmpBob[frame], Resizer(bmpBob[frame], Bob->start.x, Bob->start.y));
        }
        if (!vJellies.empty())
        {
            for (int i = 0; i < vJellies.size(); ++i)
            {
                int frame = Bob->GetFrame();
                Draw->DrawBitmap(bmpJelly[frame], Resizer(bmpJelly[frame], vJellies[i]->start.x, vJellies[i]->start.y));
            }
        }
        if (!vEvils.empty())
        {
            for (int i = 0; i < vEvils.size(); ++i)
            {
                int frame = vEvils[i]->GetFrame();

                switch (vEvils[i]->GetType())
                {
                case types::evil1:
                    if (vEvils[i]->dir == dirs::left)Draw->DrawBitmap(bmpEvil1L[frame],
                        Resizer(bmpEvil1L[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    else Draw->DrawBitmap(bmpEvil1R[frame], Resizer(bmpEvil1R[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case types::evil2:
                    if (vEvils[i]->dir == dirs::left)Draw->DrawBitmap(bmpEvil2L[frame],
                        Resizer(bmpEvil2L[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    else Draw->DrawBitmap(bmpEvil2R[frame], Resizer(bmpEvil2R[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case types::evil3:
                    if (vEvils[i]->dir == dirs::left)Draw->DrawBitmap(bmpEvil3L[frame],
                        Resizer(bmpEvil3L[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    else Draw->DrawBitmap(bmpEvil3R[frame], Resizer(bmpEvil3R[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case types::evil4:
                    if (vEvils[i]->dir == dirs::left)Draw->DrawBitmap(bmpEvil4L[frame],
                        Resizer(bmpEvil4L[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    else Draw->DrawBitmap(bmpEvil4R[frame], Resizer(bmpEvil4R[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case types::evil5:
                    if (vEvils[i]->dir == dirs::left)Draw->DrawBitmap(bmpEvil5L[frame],
                        Resizer(bmpEvil5L[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    else Draw->DrawBitmap(bmpEvil5R[frame], Resizer(bmpEvil5R[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case types::evil6:
                    if (vEvils[i]->dir == dirs::left)Draw->DrawBitmap(bmpEvil6L[frame],
                        Resizer(bmpEvil6L[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    else Draw->DrawBitmap(bmpEvil6R[frame], Resizer(bmpEvil6R[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case types::evil7:
                    if (vEvils[i]->dir == dirs::left)Draw->DrawBitmap(bmpEvil7L[frame],
                        Resizer(bmpEvil7L[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    else Draw->DrawBitmap(bmpEvil7R[frame], Resizer(bmpEvil7R[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case types::evil8:
                    if (vEvils[i]->dir == dirs::left)Draw->DrawBitmap(bmpEvil8L[frame],
                        Resizer(bmpEvil8L[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    else Draw->DrawBitmap(bmpEvil8R[frame], Resizer(bmpEvil8R[frame], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;
                }
            }
        }
        if (!vSeaGrass.empty())
        {
            for (int i = 0; i < vSeaGrass.size(); ++i)
                Draw->DrawBitmap(bmpGrass[vSeaGrass[i]->GetFrame()], D2D1::RectF(vSeaGrass[i]->start.x, vSeaGrass[i]->start.y,
                    vSeaGrass[i]->end.x, vSeaGrass[i]->end.y));
        }
        if (!vBubbles.empty())
        {
            for (int i = 0; i < vBubbles.size(); ++i)
                Draw->DrawBitmap(bmpBubbles[vBubbles[i]->GetFrame()], D2D1::RectF(vBubbles[i]->start.x, vBubbles[i]->start.y,
                    vBubbles[i]->end.x, vBubbles[i]->end.y));
        }

        Draw->EndDraw();

    }

    std::remove(tmp_file);
    ReleaseResources();
    return (int) bMsg.wParam;
}