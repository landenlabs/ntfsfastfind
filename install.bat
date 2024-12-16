@echo off

set prog=NTFSfastFind
 

echo "msbuild=%msbuild%"
if not exist "%msbuild%" (
echo Fall back msbuild not found at "%msbuild%"
set msbuild=F:\opt\VisualStudio\2022\Preview\MSBuild\Current\Bin\MSBuild.exe
)
echo "Msbuild=%msbuild%"


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
@echo Copy Release to d:\opt\bin2
copy x64\Release\%prog%.exe d:\opt\bin2\%prog%.exe

@echo.
@echo Compare md5 hash
ld -a -ph %prog%-ms\x64\Release\%prog%.exe d:\opt\bin2\%prog%.exe

:_end
