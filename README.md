a graceful terminating console application for windows

This shows how an console application can graceful terminate itself upon receiving a CTRL+C.

This should be compiled with [GCC](https://gcc.gnu.org/) from [MinGW-w64](https://mingw-w64.org/). From a [MSYS2](http://www.msys2.org/) (available as a [chocolatey package](https://chocolatey.org/packages/msys2) too) console run:

```bash
pacman --noconfirm -Sy mingw-w64-x86_64-gcc make zip
export PATH="/mingw64/bin:$PATH"
make
```