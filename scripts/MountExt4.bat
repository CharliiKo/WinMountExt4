@echo off
chcp 65001 >nul
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [错误] 请右键点击，以管理员身份运行。
    pause
    exit /b
)

echo =================================================
echo      WSL2 ext4 挂载工具 - 适用于 Windows 10/11
echo =================================================

:: 1. 显示物理磁盘
echo [步骤 1] 正在获取物理磁盘列表...
powershell -NoProfile -Command "[Console]::OutputEncoding = [System.Text.Encoding]::UTF8; Get-CimInstance -query 'SELECT * from Win32_DiskDrive' | Select-Object DeviceID, Model, Size"
echo.

set /p disk_num="请输入磁盘编号 (例如 2): "
set disk_path=\\.\PHYSICALDRIVE%disk_num%

:: 2. 尝试附加磁盘
echo.
echo [步骤 2] 正在附加磁盘 %disk_path%...
wsl --mount %disk_path% --bare 2>nul

:: 3. 列出该磁盘下的所有分区
echo.
echo [步骤 3] 探测到该磁盘下的分区如下:
echo -----------------------------------------------
:: 动态获取该磁盘在 WSL 中的设备名并列出分区
wsl -u root -e sh -c "lsblk -p -o NAME,FSTYPE,SIZE,MOUNTPOINT | grep -E 'NAME|sd|nvme'"
echo -----------------------------------------------
echo [提示] 请根据上面列表中的分区路径，输入你想挂载的ext4分区路径 (例如 /dev/sda1)。
echo.

set /p part_path="请输入你想挂载的分区路径 (例如 /dev/sda1): "

:: 4. 执行挂载
echo.
echo [步骤 4] 正在尝试挂载 %part_path% ...
wsl -u root -e sh -c "mkdir -p /mnt/wsl/ext4_disk && mount %part_path% /mnt/wsl/ext4_disk"

:: 5. 检查挂载结果
if %errorLevel% == 0 (
    echo.
    echo [成功] 挂载完成！
    echo -----------------------------------------------
    echo 访问路径: \\wsl.localhost\Ubuntu\mnt\wsl\ext4_disk
    echo -----------------------------------------------
) else (
    echo.
    echo [失败] 挂载出错。
    echo 建议检查：该分区是否真的为 ext4 格式？
    echo 尝试运行: wsl -u root -e dmesg ^| tail -n 20 查看底层错误。
)

pause