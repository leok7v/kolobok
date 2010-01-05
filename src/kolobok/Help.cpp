#include "resource.h"
#include "Help.h"
#include "Application.h"
#include "Controls.h"
#include "Skin.h"

static int CALLBACK helpDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

Help Help::help;

static void helpAbout() {
    if (Help::help.dlgAbout != null) return;
    Help::help.dlgAbout = Controls::createDialog(IDD_MB, helpDlgProc);
    assert(Help::help.dlgAbout != null);
    SetDlgItemText(Help::help.dlgAbout, IDC_VERSION, main.getVersion());
}

void Help::eventReceived(const Message& m) {
    if (ustrcmp(m.id, L"commandHelpAbout") == 0)
        helpAbout();
    else if (ustrcmp(m.id, L"updateCommandsState") == 0) {
        HashMap& map = *(HashMap*)dynamic_cast<const HashMap*>(m.param);
        map.put(L"commandHelpAbout", dlgAbout == null);
        map.put(L"commandButtonOK", dlgAbout != null);
    }
    else if (ustrcmp(m.id, L"commandButtonOK") == 0 && dlgAbout != null) {
        PostMessage(dlgAbout, WM_COMMAND, IDOK, 0);
    }
}

int CALLBACK helpDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_INITDIALOG: {
            SetProp(hwnd, L"dialog", (void*)true);
            SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(hwnd, 0, (255 * 90) / 100, LWA_ALPHA);
            return true;
        }
        case WM_DESTROY: {
            Help::help.dlgAbout = null;
            EnableWindow(getApplicationMainWindow(), true);
            break;
        }
    }
    return false;
}

