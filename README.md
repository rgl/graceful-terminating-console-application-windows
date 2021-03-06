# About

[![Build status](https://github.com/rgl/graceful-terminating-console-application-windows/workflows/Build/badge.svg)](https://github.com/rgl/graceful-terminating-console-application-windows/actions?query=workflow%3ABuild)

This is an example graceful terminating console application for Windows.

This shows how an console application can graceful terminate itself upon receiving a CTRL+C/CTRL+BREAK/CTRL_CLOSE_EVENT/CTRL_LOGOFF_EVENT/CTRL_SHUTDOWN_EVENT.

This should be compiled with [GCC](https://gcc.gnu.org/) from [MinGW-w64](https://mingw-w64.org/). From a [MSYS2](http://www.msys2.org/) (available as a [chocolatey package](https://chocolatey.org/packages/msys2) too) console run:

```bash
pacman --noconfirm -Sy mingw-w64-x86_64-gcc make zip
export PATH="/mingw64/bin:$PATH"
make
```

## Reference

* [Windows Console](https://docs.microsoft.com/en-us/windows/console/consoles)
* [Console CTRL+C and CTRL+BREAK Signals](https://docs.microsoft.com/en-us/windows/console/ctrl-c-and-ctrl-break-signals)
* [SetConsoleCtrlHandler function](https://docs.microsoft.com/en-us/windows/console/setconsolectrlhandler)
* [HandlerRoutine callback function](https://docs.microsoft.com/en-us/windows/console/handlerroutine)
