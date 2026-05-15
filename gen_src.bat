@echo off
setlocal enabledelayedexpansion

:: 결과 파일명 설정
set OUTPUT_FILE=sources.txt

:: 기존 결과 파일 삭제 (실행할 때마다 새로 생성)
if exist %OUTPUT_FILE% del %OUTPUT_FILE%

echo Start merging files...
echo.

:: 1. \include 및 \test 디렉토리 아래의 cpp, h, hpp 파일 찾아서 합치기
:: /r은 하위 폴더까지 검색
for /r %%f in (*.cpp *.h *.hpp) do (
    
    :: 결과 파일(sources.txt) 자체는 제외
    echo "%%f" | findstr /i "%OUTPUT_FILE%" >nul
    if errorlevel 1 (
        echo Processing: %%f
        
        :: 서식: ///----- /// <디렉토리>\파일명
        echo ///-----
	echo /// %%f >> %OUTPUT_FILE%
	echo ///-----	
        echo. >> %OUTPUT_FILE%
        
        :: 파일 내용물 추가
        type "%%f" >> %OUTPUT_FILE%
        
        :: 줄바꿈 추가 (다음 파일과 붙지 않게)
        echo. >> %OUTPUT_FILE%
        echo. >> %OUTPUT_FILE%
    )
)

echo.
echo Merging completed. Output file: %OUTPUT_FILE%
pause
