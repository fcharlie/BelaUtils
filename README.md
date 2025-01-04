# BelaUtils

[![license badge](https://img.shields.io/github/license/fcharlie/belautils.svg)](LICENSE)
[![Master Branch Status](https://github.com/fcharlie/belautils/workflows/CI/badge.svg)](https://github.com/fcharlie/belautils/actions)
[![Latest Release Downloads](https://img.shields.io/github/downloads/fcharlie/belautils/latest/total.svg)](https://github.com/fcharlie/belautils/releases/latest)
[![Total Downloads](https://img.shields.io/github/downloads/fcharlie/belautils/total.svg)](https://github.com/fcharlie/belautils/releases)

Tools reimplemented using Bela library


## Bona - Modern and interesting file format viewer

A modern and interesting file format viewer

Feature:

+   Support multiple file format detection
+   Support parsing zip container file format
+   Support the correct conversion of zip file name encoding after detection
+   Support analysis of PE/ELF/MachO binary format, dependencies, import/export symbols, etc.

```txt
bona - Modern and interesting file format viewer
Usage: bona [option]... [file]...
  -h|--help        Show usage text and quit
  -v|--version     Show version number and quit
  -V|--verbose     Make the operation more talkative
  -f|--full        Full mode, view more detailed information of the file.
  -j|--json        Format and output file information into JSON.
```

![](https://s3.ax1x.com/2020/12/26/r4Rpex.png)

![](https://s3.ax1x.com/2020/12/26/r4ReOI.png)

## Caelum - PE Simple Analysis Tool

Caelum is derived from [PEAnalyzer](https://github.com/fcharlie/PEAnalyzer/), which is a GUI PE analysis tool that can analyze the structure, dependency and other information of PE files.

![](./docs/images/caelum.png)


## Kisasum Hash Utilities

Kisasum is derived from [Kismet](https://github.com/fcharlie/Kismet) and is a hash calculation tool that supports SHA228/SHA256/SHA384/SHA512/SHA3/BAKE3/KangarooTwelve/BLAKE2b/BLAKE2s.

CLI usage:

```shell
kisasum -a BLAKE3 path/to/file
```

GUI Snapshot:

![](./docs/images/kisasum-ui.png)


## Krycekium MSI unpacker

Krycekium is derived from  [Krycekium](https://github.com/fcharlie/Krycekium) and is an MSI installation package decompression tool.

![](./docs/images/krycekium.png)

## hastyhex

A faster hex dumper fork from [https://github.com/skeeto/hastyhex](https://github.com/skeeto/hastyhex). 

![](./docs/images/hastyhex.png)

## Download

Download the latest version of Baulk: [https://github.com/fcharlie/belautils/releases/latest](https://github.com/fcharlie/belautils/releases/latest)

or

```powershell
baulk install belautils
```

<div>Kisasum UI Icons made by <a href="https://www.flaticon.com/authors/smashicons" title="Smashicons">Smashicons</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>