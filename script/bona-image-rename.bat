@echo off
pwsh -NoProfile -NoLogo -ExecutionPolicy unrestricted -File "%~dp0bona-image-rename.ps1" %*
