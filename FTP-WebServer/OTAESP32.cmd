if x%1==x goto:help
set IP=%1
set BinFile1=%CD%\build\esp32.esp32.esp32\SDWebServer.ino.bin
set BinFile2=%CD%\.pio\build\esp32dev\firmware.bin
if exist %BinFile1% set BinFile=%BinFile1% & echo Arduino IDE
if exist %BinFile2% set BinFile=%BinFile2% & echo PlatformIO
if NOT exist %BinFile% ( echo Fireware file not found & goto :EOF )
FOR /D %%D IN (%LOCALAPPDATA%\Arduino15\packages\esp32\hardware\esp32\*) DO (
 if exist %%D\tools\espota.py set espota=%%D\tools\espota.py
 goto gotespota
)
:gotespota
echo "%espota%"
python %espota% -i %IP% -p 3232 -a admin -f %BinFile%
goto :EOF
:help
Echo use %0 <IPadres>
:EOF
