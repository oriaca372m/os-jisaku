ゼロからのOS自作入門(http://zero.osdev.jp/) の写経

# Arch Linuxでのビルド方法

## 必要パッケージ
- base-devel
- clang
- lld
- nasm
- qemu
- acpica
- edk2-ovmf
- git
- cmake
- ninja
- docker
- 抜けがあるかも

## EDK2のインストール
``` bash
# このリポジトリ外の適当なディレクトリで
$ git clone 'https://github.com/tianocore/edk2'
$ cd edk2
$ git switch --detach edk2-stable202105
$ git submodule update --init
$ (cd BaseTools/Source/C/BrotliCompress/brotli && git apply このリポジトリのルート/brotli_warning_fix.patch)
$ make -C BaseTools
```

## ブートローダーのビルド

### 初回
``` bash
# EDK2のインストール先で
$ ln -s このリポジトリのルート/MikanLoaderPkg
$ . ./edksetup.sh
```

`Conf/target.txt` を次のように編集

| 変数 | 値 |
| --- | --- |
| ACTIVE_PLATFORM | MikanLoaderPkg/MikanLoaderPkg.dsc |
| TARGET | DEBUG |
| TARGET_ARCH | X64 |
| TOOL_CHAIN_TAG | CLANG38 |

``` bash
$ build
```

### 2回目以降
``` bash
# EDK2のインストール先で
$ . ./edksetup.sh
$ build
```

## カーネルで使用する標準ライブラリのビルド
``` bash
# このリポジトリのルートで
$ cd stdlib
$ ./build.sh
```

## カーネルのビルド
``` bash
# このリポジトリのルートで
$ ./build-kernel.sh
```

## 実行
``` bash
# このリポジトリのルートで
$ ./run_qemu.sh EDK2のインストール先/Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi
```

# ライセンス
このソフトウェアはApache License Version 2.0(全文は`LICENSE`を参照)の元で配布されているMikanOS (https://github.com/uchan-nos/mikanos) を、コードのフォーマットなどを変更しながら書き写したものです。

このソフトウェア自身にもApache License Version 2.0を適用します。
