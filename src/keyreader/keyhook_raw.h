// This file is loaded only once at keyhook.c
// So all the functions are safely implemented in
// the header file
#pragma once

#include "keyhook.h"
#include "keytyper.h"

#include <stdbool.h>

#ifdef _WIN32
#define _WINSOCKAPI_
#include <windows.h>

DWORD __keyhook_thread = 0;

char __to_ascii(KBDLLHOOKSTRUCT *kbstruct) {
    BYTE keyboardState[256] = {0};
    DWORD scanCode = kbstruct->scanCode;
    DWORD virtualKeyCode = kbstruct->vkCode;

    if (kbstruct->flags & LLKHF_EXTENDED)
        scanCode |= 0x100;

    HKL keyboardLayout = GetKeyboardLayout(0);

    keyboardState[VK_SHIFT] = GetKeyState(VK_SHIFT) & 0x80;
    keyboardState[VK_CAPITAL] = GetKeyState(VK_CAPITAL) & 0x01;

    UINT mappedScanCode =
        MapVirtualKeyEx(scanCode, MAPVK_VSC_TO_VK_EX, keyboardLayout);

    WCHAR unicodeChar = 0;
    ToUnicodeEx(virtualKeyCode, mappedScanCode, keyboardState, &unicodeChar, 1,
                0, keyboardLayout);

    char typedChar = (char)unicodeChar;

    return typedChar;
}

LRESULT CALLBACK keyhook_callback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_KEYUP)) {
        KBDLLHOOKSTRUCT *kbstruct = (KBDLLHOOKSTRUCT *)lParam;
        KeyHookInfo info = (KeyHookInfo)kbstruct->dwExtraInfo;
        DWORD flags = kbstruct->flags;

        if (info != KEYTYPER_KH_IGNORE) {
            DWORD virtual_code = kbstruct->vkCode;
            DWORD scan_code = kbstruct->scanCode;
            char key_char = (kbstruct->vkCode != VK_PACKET)
                                ? __to_ascii(kbstruct)
                                : (char)kbstruct->scanCode;

            KeyEvent event = (KeyEvent){
                .character = key_char,
                .is_keydown = wParam == WM_KEYDOWN,
                .is_ctrldown = keytyper_is_pressed(VK_CONTROL),
            };

            if (keyhook_handle_event(event))
                return true;
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK mousehook_callback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION &&
        (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN ||
         wParam == WM_MBUTTONDOWN)) {
        keyhook_reset();
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void keyhook_raw_run() {
    __keyhook_thread = GetCurrentThreadId();

    HHOOK khook = SetWindowsHookEx(WH_KEYBOARD_LL, keyhook_callback, NULL, 0);
    HHOOK mhook = SetWindowsHookEx(WH_MOUSE_LL, mousehook_callback, NULL, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_USER + 1) {
            break; // Exit the loop when a quit message is received
        }
    }

    UnhookWindowsHookEx(mhook);
    UnhookWindowsHookEx(khook);
}

void keyhook_raw_quit() {
    PostThreadMessage(__keyhook_thread, WM_USER + 1, 0, 0);
}

#else
#error "Key hook not implemented for NON WINDOWS PLATFORMS"
#endif