# WinMountExt4
A little bat code run on windows 10/11 to mount your ext4 disk via WSL2.

1. Install WSL2.
2. Run with powershell(Administrator) `./MountExt4.bat` to mount your disk,
3. Run with powershell(Administrator) `./UnmountExt4.bat` to unmount your disk.

You are allowed to access the mounted disk in **windows exploer** directory.
for default the `\\wsl.localhost\Ubuntu\mnt\wsl\ext4_disk`.

Enjoy yourself!🦖

TODO: 
1. Add an easy-to-use GUI based on Qt.
2. Startup when user login function.
