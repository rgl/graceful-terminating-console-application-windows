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

#define _WIN32_WINNT 0x0600
#define WINVER _WIN32_WINNT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <io.h>

void LOG(const char *format, ...) {
    time_t t;
    time(&t);
    char buffer[128];
    strftime(buffer, 128, "%Y-%m-%d %H:%M:%S ", localtime(&t));
    int l = strlen(buffer);
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer+l, 128-l, format, args);
    va_end(args);
    puts(buffer);
    l = strlen(buffer);
    buffer[l++] = '\n';
    buffer[l] = 0;
    FILE *log = fopen("graceful-terminating-console-application-windows.log", "a+");
    fputs(buffer, log);
    fclose(log);
}

HANDLE g_stopEvent = NULL;

// NB this handler runs in a dedicated thread.
BOOL WINAPI consoleControlHandler(DWORD ctrlType) {
    LPCSTR ctrlTypeName;
    #define HANDLE_CONSOLE_CONTROL_EVENT(e) case e: ctrlTypeName = #e; break;
    switch (ctrlType) {
        HANDLE_CONSOLE_CONTROL_EVENT(CTRL_C_EVENT)
        HANDLE_CONSOLE_CONTROL_EVENT(CTRL_BREAK_EVENT)
        HANDLE_CONSOLE_CONTROL_EVENT(CTRL_CLOSE_EVENT)
        HANDLE_CONSOLE_CONTROL_EVENT(CTRL_LOGOFF_EVENT)
        HANDLE_CONSOLE_CONTROL_EVENT(CTRL_SHUTDOWN_EVENT)
        default:
            return FALSE;
    }
    #undef HANDLE_CONSOLE_CONTROL_EVENT
    LOG("Received the console %s, gracefully terminating the application...", ctrlTypeName);
    SetEvent(g_stopEvent);
    switch (ctrlType) {
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            // do not return from this handler, or else, windows immediately calls TerminateProcess.
            // NB Windows will only wait for about 5 seconds before calling TerminateProcess...
            Sleep(INFINITE);
            return FALSE;
    }
    return TRUE;
}

void logFdInformation(int fd) {
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    if (h == INVALID_HANDLE_VALUE) {
        LOG("ERROR: cannot get fd information because fd %d is invalid", fd);
        return;
    }

    char *fileType = "unknown";
    DWORD ft = GetFileType(h);
    switch (ft) {
        case FILE_TYPE_CHAR:
            fileType = "console";
            break;
        case FILE_TYPE_DISK:
            fileType = "disk";
            break;
        case FILE_TYPE_PIPE:
            fileType = "pipe";
            break;
        case FILE_TYPE_REMOTE:
            fileType = "remote";
            break;
        case FILE_TYPE_UNKNOWN:
            fileType = "unknown";
            DWORD lastError = GetLastError();
            if (NO_ERROR != lastError) {
                HRESULT hr = HRESULT_FROM_WIN32(lastError);
                LOG("ERROR: Failed to GetFileType of fd %d with HRESULT 0x%x", fd, hr);
                return;
            }
            break;
        default:
            LOG("ERROR: fd %d has unknown file type %x", fd, ft);
            return;
    }

    if (ft == FILE_TYPE_CHAR) {
        LOG("fd %d is a %s", fd, fileType);
    } else {
        DWORD size = sizeof(FILE_NAME_INFO) + sizeof(WCHAR)*(MAX_PATH+1);
        FILE_NAME_INFO *fileNameInfo = malloc(size);
        if (!GetFileInformationByHandleEx(h, FileNameInfo, fileNameInfo, size)) {
            free(fileNameInfo);
            HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
            LOG("ERROR: Failed to GetFileInformationByHandleEx with HRESULT 0x%x", hr);
            return;
        }
        fileNameInfo->FileName[fileNameInfo->FileNameLength/sizeof(WCHAR)] = 0;
        LOG("fd %d is a %s at %ls", fd, fileType, fileNameInfo->FileName);
        free(fileNameInfo);
    }
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
        LOG("ERROR: Failed to create stop event with HRESULT 0x%x", hr);
        goto cleanup;
    }

    if (!SetConsoleCtrlHandler(consoleControlHandler, TRUE)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG("ERROR: Failed to set console control handler with HRESULT 0x%x", hr);
        goto cleanup;
    }

    LOG("Running (pid=%d)... press CTRL+C to terminate.", GetCurrentProcessId());

    // log stdin, stdout and stderr file descriptor information.
    for (int fd = 0; fd < 3; ++fd) {
        logFdInformation(fd);
    }

    if (WAIT_OBJECT_0 != WaitForSingleObject(g_stopEvent, INFINITE)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG("ERROR: Failed to wait for the stop event with HREAULT 0x%x", hr);
        goto cleanup;
    }

    for (; n; --n) {
        LOG("Gracefully terminating the application in T-%d...", n);
        Sleep(1000);
    }

    LOG("Bye bye...");

cleanup:
    if (g_stopEvent) {
        CloseHandle(g_stopEvent);
    }
    return hr;
}
