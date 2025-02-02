@echo off

set prog=NTFSfastFind
 

set dstdir=%bindir%
if not exist "%dstdir%" (
 if exist c:\opt\bin  set dstdir=c:\opt\bin
 if exist d:\opt\bin  set dstdir=d:\opt\bin
)
echo "msbuild=%msbuild%"
if not exist "%msbuild%" (
echo Fall back msbuild not found at "%msbuild%"
set msbuild=F:\opt\VisualStudio\2022\Preview\MSBuild\Current\Bin\MSBuild.exe
)

@echo Clean %proj% 
rmdir /s x64 2> nul

@echo.
@echo Build release target
"%msbuild%" %prog%.sln -p:Configuration="Release";Platform=x64 -verbosity:minimal  -detailedSummary:True 
 

@echo.
@echo ---- Build done 
if not exist "x64\Release\%prog%.exe" (
   echo Failed to build x64\Release\%prog%.exe
   goto _end
)

@echo.
@echo Copy Release to %dstdir%
copy x64\Release\%prog%.exe %dstdir%\%prog%.exe

@echo.
@echo Compare md5 hash
ld -a -ph x64\Release\%prog%.exe %dstdir%\%prog%.exe

@rem play happy tone
rundll32.exe cmdext.dll,MessageBeepStub
rundll32 user32.dll,MessageBeep

:_end
