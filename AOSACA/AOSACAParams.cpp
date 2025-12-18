#include "stdafx.h"
#include "AOSACAParams.h"
#include <afxwin.h>

// ============================================================================
// Global AOSACA Error / Info Message Handling
// ============================================================================
//
// This version ensures that all message boxes are shown in the foreground
// and are owned by the main dialog window (ghDlg).
// It replaces any inline definitions to prevent multiple instances of the function.
//

// Forward declarations to avoid circular dependency

int AOSACAParams::ShowError(int msgtype)
{
    if (ghDlg == NULL)
    {
        AfxMessageBox(_T("AOSACA Error: Main window handle not set."), MB_OK | MB_ICONERROR);
        return IDCANCEL;
    }

    // allocate and prepare data for main-thread handling
    SHOWERRORDATA* pData = new SHOWERRORDATA{ msgtype, &g_stAppErrBuff };

    ::PostMessage(ghDlg, WM_APP_SHOW_ERROR_MESSAGE, 0, reinterpret_cast<LPARAM>(pData));
    return IDOK;
}