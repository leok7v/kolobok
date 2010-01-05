#include "tiny.h"

/* Ole DoDragDrop "swallows" messages. After extensive study of it
   the simplest solution is just to implement and use its' equvivalent
   that does not.
*/

struct DropTargetNode {
    HWND hwnd; // target
    IDropTarget* dt;
    DropTargetNode* prev;
    DropTargetNode* next;
};

struct TrackerWindowInfo {
    HRESULT hr;
    IDataObject* object;
    IDropSource* dropSource;
    IDropTarget* dragTarget;
    DWORD  allowedEffects;
    DWORD* effect;
    HWND hwndTarget;
    HWND hwndDragTarget;
    bool done;
    bool esc;
};

static const uchar DRAGTRACKERCLASS[] = L"DnDTracker";
static DropTargetNode* targets = null;

static int getButtonState() {
    byte ks[256] = { 0 };
    GetKeyboardState(ks);
    int keyMask = 0;
    if ((ks[VK_SHIFT]   & 0x80) != 0) keyMask |= MK_SHIFT;
    if ((ks[VK_CONTROL] & 0x80) != 0) keyMask |= MK_CONTROL;
    if ((ks[VK_LBUTTON] & 0x80) != 0) keyMask |= MK_LBUTTON;
    if ((ks[VK_RBUTTON] & 0x80) != 0) keyMask |= MK_RBUTTON;
    if ((ks[VK_MBUTTON] & 0x80) != 0) keyMask |= MK_MBUTTON;
    return keyMask;
}

static void insertDropTarget(DropTargetNode* nodeToAdd) {
    DropTargetNode* node = targets;
    DropTargetNode** prev = &targets;
    while (node != null) {
        if (nodeToAdd->hwnd < node->hwnd) {
            prev = &node->prev;
            node =  node->prev;
        }
        else if (nodeToAdd->hwnd > node->hwnd) {
            prev = &node->next;
            node =  node->next;
        }
        else {
            assert(false);
            return;
        }
    }
    assert(node == null);
    assert(*prev == null);
    *prev = nodeToAdd;
}

static DropTargetNode* extractDropTarget(HWND hwndOfTarget) {
    DropTargetNode* node = targets;
    DropTargetNode** prev = &targets;
    while (node != null) {
        if (hwndOfTarget < node->hwnd) {
            prev = &node->prev;
            node =  node->prev;
        }
        else if (hwndOfTarget > node->hwnd) {
            prev = &node->next;
            node =  node->next;
        }
        else {
            assert(*prev == node);
            *prev = node->prev;
            if (node->next != null) insertDropTarget(node->next);
            node->next = null;
            node->prev = null;
            return node;
        }
    }
    return null;
}

static DropTargetNode* findDropTarget(HWND hwndOfTarget) {
    DropTargetNode* node = targets;
    while (node != null) {
        if (hwndOfTarget < node->hwnd) node =  node->prev;
        else if (hwndOfTarget > node->hwnd) node =  node->next;
        else return node;
    }
    return null;
}

static void trackMouseMove(TrackerWindowInfo* trackerInfo, POINT pt, DWORD keyState) {
    HRESULT hr = S_OK;
    HWND newTarget = WindowFromPoint(pt);
    *trackerInfo->effect = trackerInfo->allowedEffects;
    if (trackerInfo->dragTarget != null && trackerInfo->hwndTarget == newTarget) {
        POINTL mouse = { pt.x, pt.y };
        trackerInfo->dragTarget->DragOver(keyState, mouse, trackerInfo->effect);
    }
    else {
        DropTargetNode* newDropTargetNode = 0;
        if (trackerInfo->dragTarget != null) {
            trackerInfo->dragTarget->DragLeave();
        }
        if (newTarget != 0) {
            HWND hwnd = newTarget;
            trackerInfo->hwndTarget = newTarget;
            do { newDropTargetNode = findDropTarget(hwnd); } 
            while (!newDropTargetNode && (hwnd = GetParent(hwnd)) != 0);
            if (hwnd != null) newTarget = hwnd;
            trackerInfo->hwndDragTarget = newTarget;
            trackerInfo->dragTarget = newDropTargetNode != null ? newDropTargetNode->dt : null;
            if (trackerInfo->dragTarget != null) {
                POINTL mouse = { pt.x, pt.y };
                trackerInfo->dragTarget->DragEnter(trackerInfo->object,
                                                   keyState, mouse, trackerInfo->effect);
            }
        }
        else {
            trackerInfo->hwndDragTarget = null;
            trackerInfo->hwndTarget = null;
            trackerInfo->dragTarget = null;
        }
    }
    if (trackerInfo->dragTarget == null) *trackerInfo->effect = DROPEFFECT_NONE;
    hr = trackerInfo->dropSource->GiveFeedback(*trackerInfo->effect);
    if (hr == DRAGDROP_S_USEDEFAULTCURSORS) {
        int id = 1;
        if (*trackerInfo->effect & DROPEFFECT_MOVE) id = 2;
        else if (*trackerInfo->effect & DROPEFFECT_COPY) id = 3;
        else if (*trackerInfo->effect & DROPEFFECT_LINK) id = 4;
        HINSTANCE ole32 = GetModuleHandle(L"ole32.dll");
        HCURSOR crs = LoadCursor(ole32, MAKEINTRESOURCE(id));
        if (crs != null) SetCursor(crs);
    }
}

static void trackStateChange(TrackerWindowInfo* trackerInfo, POINT pt, DWORD keyState) {
    trackerInfo->hr = trackerInfo->dropSource->QueryContinueDrag(trackerInfo->esc, keyState);
    if (trackerInfo->hr != S_OK) {
        trackerInfo->done = true;
        ReleaseCapture();
        if (trackerInfo->dragTarget != null) {
            switch (trackerInfo->hr) {
                case DRAGDROP_S_DROP: {
                    POINTL mouse = { pt.x, pt.y };
                    trackerInfo->dragTarget->Drop(trackerInfo->object,
                                                  keyState, mouse, trackerInfo->effect);
                    break;
                }
                case DRAGDROP_S_CANCEL: {
                    trackerInfo->dragTarget->DragLeave();
                    *trackerInfo->effect = DROPEFFECT_NONE;
                    break;
                }
            }
        }
    }
}

static LRESULT CALLBACK dragTrackerWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg)
    {
        case WM_CREATE: {
            LPCREATESTRUCT createStruct = (LPCREATESTRUCT)lp;
            SetWindowLong(hwnd, 0, (long)createStruct->lpCreateParams);
            break;
        }
        case WM_MOUSEMOVE: {
            TrackerWindowInfo* trackerInfo = (TrackerWindowInfo*)GetWindowLong(hwnd, 0);
            POINT pt = { LOWORD(lp), HIWORD(lp) };
            ClientToScreen(hwnd, &pt);
            trackMouseMove(trackerInfo, pt, wp);
            break;
        }
        case WM_LBUTTONUP: case WM_MBUTTONUP: case WM_RBUTTONUP: 
        case WM_LBUTTONDOWN: case WM_MBUTTONDOWN: case WM_RBUTTONDOWN: {
            TrackerWindowInfo* trackerInfo = (TrackerWindowInfo*)GetWindowLong(hwnd, 0);
            POINT pt = { LOWORD(lp), HIWORD(lp) };
            ClientToScreen(hwnd, &pt);
            trackStateChange(trackerInfo, pt, wp);
            break;
        }
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

HRESULT DnD::registerDragDrop(HWND hwnd, LPDROPTARGET pDropTarget) {
//  trace(L"registerDragDrop(%p, %p)\n", hwnd, pDropTarget);
    if (pDropTarget == null) return E_INVALIDARG;
    DropTargetNode* dropTargetInfo = findDropTarget(hwnd);
    if (dropTargetInfo != null) return DRAGDROP_E_ALREADYREGISTERED;
    dropTargetInfo = new DropTargetNode();
    if (dropTargetInfo == null) return E_OUTOFMEMORY;
    dropTargetInfo->hwnd = hwnd;
    dropTargetInfo->prev = null;
    dropTargetInfo->next = null;
    dropTargetInfo->dt = pDropTarget;
    dropTargetInfo->dt->AddRef();
    insertDropTarget(dropTargetInfo);
    return S_OK;
}

HRESULT DnD::revokeDragDrop(HWND hwnd) {
//  trace(L"revokeDragDrop(%p)\n", hwnd);
    DropTargetNode* dropTargetInfo = extractDropTarget(hwnd);
    if (dropTargetInfo == null) return DRAGDROP_E_NOTREGISTERED;
    dropTargetInfo->dt->Release();
    delete dropTargetInfo;
    return S_OK;
}

HRESULT DnD::doDragDrop(IDataObject *object,
                        IDropSource* dropSource,
                        DWORD allowedEffects,
                        DWORD* effect) {
//  trace(L"doDragDrop(%p, %p)\n", object, dropSource);
    if (dropSource == null) return E_INVALIDARG;
    TrackerWindowInfo trackerInfo = {0};
    trackerInfo.object = object;
    trackerInfo.dropSource = dropSource;
    trackerInfo.allowedEffects = allowedEffects;
    trackerInfo.effect = effect;
    trackerInfo.done = false;
    trackerInfo.esc = false;
    trackerInfo.hwndDragTarget = 0;
    trackerInfo.hwndTarget = 0;
    trackerInfo.dragTarget = 0;
    MSG msg = {0};
    HWND hwndTrackWindow = CreateWindow(DRAGTRACKERCLASS, L"DnDTracker", WS_POPUP,
                                        -1, -1, -1, -1, 0, 0, 0, &trackerInfo);
    if (hwndTrackWindow != 0) {
        SetCapture(hwndTrackWindow);
        while (!trackerInfo.done && GetMessage(&msg, 0, 0, 0)) {
            if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) {
                if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
                    trackerInfo.esc = true;
                }
                trackStateChange(&trackerInfo, msg.pt, getButtonState());
            }
            else {
                DispatchMessage(&msg);
            }
        }
        DestroyWindow(hwndTrackWindow);
        return trackerInfo.hr;
    }
    return E_FAIL;
}

void DnD::init() {
    WNDCLASS wndClass = {0};
    wndClass.style = CS_GLOBALCLASS;
    wndClass.lpfnWndProc = dragTrackerWindowProc;
    wndClass.cbWndExtra = sizeof(TrackerWindowInfo*);
    wndClass.lpszClassName = DRAGTRACKERCLASS;
    RegisterClass(&wndClass);
}

void DnD::fini() {
    while (targets != null) RevokeDragDrop(targets->hwnd);
}
