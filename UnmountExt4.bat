@echo off
chcp 65001 >nul
:: 设置 CMD 窗口为 UTF-8 编码，防止脚本内的中文说明乱码

net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [错误] 请以管理员身份运行此脚本。
    pause
    exit /b
)

echo =================================================
echo      WSL2 ext4 卸载工具 - 适用于 Windows 10/11
echo =================================================

echo 正在获取物理磁盘列表...
powershell -NoProfile -Command "[Console]::OutputEncoding = [System.Text.Encoding]::UTF8; Get-CimInstance -query 'SELECT * from Win32_DiskDrive' | Select-Object DeviceID, Model, Size"
echo.

set /p disk_num="请输入要卸载的磁盘编号 (如 2): "
wsl --unmount \\.\PHYSICALDRIVE%disk_num%
echo 磁盘 \\.\PHYSICALDRIVE%disk_num% 已安全弹出。
pause