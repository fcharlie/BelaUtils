#!/usr/bin/env pwsh
param(
    [Parameter(Position = 0)]
    [string]$Path
)

if ($null -eq $Path) {
    Write-Host -ForegroundColor Red "missing input path"   
    exit 1
}

$items = Get-ChildItem -Path $Path -File -ErrorAction SilentlyContinue
if ($null -eq $items) {
    Write-Host -ForegroundColor Red "no file found"   
    exit 0
}

$mimeExtension = @{
    "image/png"                 = ".png";
    "image/jp2"                 = ".jp2";
    "image/gif"                 = ".gif";
    "image/jpeg"                = ".jpg";
    "image/webp"                = ".webp";
    "image/x-canon-cr2"         = ".cr2";
    "image/tiff"                = ".tiff";
    "image/bmp"                 = ".bmp";
    "image/vnd.adobe.photoshop" = ".psd";
    "image/vnd.ms-photo"        = ".jxr";
    "image/vnd.microsoft.icon"  = ".ico";
    "image/heif"                = ".heif";
    "image/avif"                = ".avif";
}

foreach ($item in $items) {
    Write-Host -ForegroundColor Yellow "bona analysis: $($item.BaseName)"
    $obj = bona -j $item.FullName | ConvertFrom-Json -ErrorAction SilentlyContinue
    if ($null -eq $obj) {
        Write-Host -ForegroundColor Red "analysis $($item.FullName) failed"   
        continue
    }
    if ($null -ne $obj.code -and $obj.code -ne 0) {
        Write-Host -ForegroundColor Red "analysis $($item.FullName) failed. code: $($obj.code) message: $($obj.message)"   
        continue
    }
    Write-Host "detect $($obj.path) mime: $($obj.mime)"
    $extension = $mimeExtension[$obj.mime]
    if ($null -eq $extension) {
        Write-Host -ForegroundColor Yellow "skip $($obj.mime)"
        continue
    }
    $originExtension = $item.Extension.ToLower()
    if ($originExtension -ne $extension) {
        $NewName = Join-Path $item.DirectoryName -ChildPath "$($item.BaseName)$extension"
        Write-Host -ForegroundColor Yellow "extension $originExtension not match real $extension"
        Write-Host -ForegroundColor Yellow "newname $NewName"
        Rename-Item $item.FullName -NewName $NewName
    }
}