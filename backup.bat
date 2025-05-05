@echo off

@rem
@rem  Backup to Dropbox using 7z
@rem

set DSTNAME=ntfsfastfind
set DSTDIR=C:\opt\Dropbox\backups\cpp
set DSTDIR=f:\ssd-dropbox\Dropbox\backups\cpp
set Z7=d:\opt\disk\7zip\7z.exe

set OUTZIP=%DSTNAME%_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.zip
set DST=%DSTDIR%\%OUTZIP%



echo %Z7% a %DST% .\ -mx0 -xr!bin -xr!obj -xr!Debug -xr!x64 -xr!.git -xr!.vs
%Z7% a %DST% .\ -mx0 -xr!bin -xr!obj -xr!Debug -xr!x64 -xr!.git -xr!.vs

echo ---- Backups ----
dir %DSTDIR%\%DSTNAME%*
