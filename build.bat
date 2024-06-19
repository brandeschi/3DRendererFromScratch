@echo off
if not exist .\build (
    mkdir .\build
)
pushd .\build
rem -Oi will turn on 'intrinsic optimizations'
rem -GR- will turn off runtime type info
rem -EHa- will turn off exception handling overhead
rem -MT will package the C runtime into our exe so we don't have to load the random dll that calls into our program. [For shipping imo]
rem -Gm- turns off any increment. builds that the compiler tries to do
set CompilerFlags= -MTd /Zi /FC -Oi -GR- -EHa- -Gm-
rem WX is supposed to represent don't compile with warnings. We will edit our warnings to only the ones we care about!
rem -Wd[warning#] is how to ignore certain warnings.
set WarningFlags=-WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4996
set SDLInclude=/I ..\vendor\SDL2\include
set SDLLinker=/LIBPATH:..\vendor\SDL2\lib\x64
cl -nologo %WarningFlags% %CompilerFlags% %SDLInclude% ..\src\main.cpp /link %SDLLinker% /SUBSYSTEM:CONSOLE shell32.lib SDL2.lib SDL2main.lib
popd
