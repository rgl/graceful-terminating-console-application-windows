/* a graceful terminating console application for windows.
*
* Copyright (c) 2017, Rui Lopes (ruilopes.com)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*   * Neither the name of Redis nor the names of its contributors may be used
*     to endorse or promote products derived from this software without
*     specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#define _WIN32_WINNT 0x0501
#define WINVER _WIN32_WINNT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>

HANDLE g_stopEvent = NULL;

BOOL WINAPI consoleControlHandler(DWORD ctrlType) {
    LPCWSTR ctrlTypeName;
    switch (ctrlType) {
        case CTRL_C_EVENT:          ctrlTypeName = L"CTRL_C_EVENT";         break;
        case CTRL_CLOSE_EVENT:      ctrlTypeName = L"CTRL_CLOSE_EVENT";     break;
        case CTRL_BREAK_EVENT:      ctrlTypeName = L"CTRL_BREAK_EVENT";     break;
        case CTRL_LOGOFF_EVENT:     ctrlTypeName = L"CTRL_LOGOFF_EVENT";    break;
        case CTRL_SHUTDOWN_EVENT:   ctrlTypeName = L"CTRL_SHUTDOWN_EVENT";  break;
        default:                    return FALSE;
    }
    wprintf(L"\n#### Received the console %s, gracefully terminating the application...\n\n", ctrlTypeName);
    SetEvent(g_stopEvent);
    return TRUE;
}

int wmain(int argc, wchar_t *argv[]) {
    HRESULT hr = S_OK;
    int n = argc == 2 ? _wtoi(argv[1]) : 10;

    g_stopEvent = CreateEvent(
        NULL,	// default security attributes
        TRUE, 	// manual-reset event
        FALSE,  // non-signaled initial state
        NULL);  // event name
    if (g_stopEvent == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"ERROR: Failed to create stop event with HRESULT 0x%x\n", hr);
        goto cleanup;
    }

    if (!SetConsoleCtrlHandler(consoleControlHandler, TRUE)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"ERROR: Failed to set console control handler with HRESULT 0x%x\n", hr);
        goto cleanup;
    }

    wprintf(L"Running... press CTRL+C to terminate.\n");

    if (WAIT_OBJECT_0 != WaitForSingleObject(g_stopEvent, INFINITE)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"ERROR: Failed to wait for the stop event with HREAULT 0x%x\n", hr);
        goto cleanup;
    }

    for (; n; --n) {
        wprintf(L"Gracefully terminating the application in T-%d...\n", n);
        Sleep(1000);
    }

    wprintf(L"Bye bye...");

cleanup:
    if (g_stopEvent) {
        CloseHandle(g_stopEvent);
    }
    return hr;
}
