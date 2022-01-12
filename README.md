# br-index

## About

This repository implements __bi-directional__ r-index (br-index).

r-index is a compressed text index which supports count(P) and locate(P).
Its size is O(r) words, whose r is the number of equal-letter runs in BWT of the text.

r-index was originally proposed in the following papers:
- Gagie, T., Navarro, G., & Prezza, N. (2018). Optimal-time text indexing in BWT-runs bounded space. In Proceedings of the Twenty-Ninth Annual ACM-SIAM Symposium on Discrete Algorithms (pp. 1459-1477). Society for Industrial and Applied Mathematics.
- Gagie, T., Navarro, G., & Prezza, N. (2020). Fully functional suffix trees and optimal text searching in BWT-runs bounded space. Journal of the ACM (JACM), 67(1), 1-54.

br-index we have proposed is achieved by using the mechanism of r-index but adding new structures. It allows for bi-directional extension of patterns.

## System Requirements

This project is based on [sdsl-lite](https://github.com/simongog/sdsl-lite) library.
Install sdsl-lite beforehand and modify variables SDSL_INCLUDE and SDSL_LIB in _CMakeLists.txt_.

## How to Use

Firstly, clone the repository. Since a submodule is used ([iutest](https://github.com/srz-zumix/iutest)), recursive cloning is necessary.
```bash
git clone --recursive https://github.com/U-Ar/br-index.git
```
In order to build, execute following commands: (This project is built by CMake)
```bash
mkdir build
cd build
cmake ..
make
```
5 executables will be created in the _build_ directory.
<dl>
	<dt>bri-build</dt>
	<dd>builds br-index on input text file.</dd>
	<dt>bri-count</dt>
	<dd>counts the number of occurrences of the given pattern using the index.</dd>
	<dt>bri-locate</dt>
	<dd>locates the occurrences of the given pattern using the index.</dd>
	<dt>bri-space</dt>
	<dd>shows the index size.</dd>
	<dt>run_tests</dt>
	<dd>runs unit tests.</dd>
</dl>

Also, you can run unit tests by
```bash
make test-bri
```

## Versions

<dl>
	<dt>br_index_naive.hpp</dt>
	<dd>Naive implementation of br-index. All samples p,j,d,pR,jR,dR,len are maintained during the search.</dd>
	<dt>br_index.hpp</dt>
	<dd>Simplified implementation of br-index (default). Only samples necessary for locate (j,d,len) are maintained.</dd>
</dl>

## 実装方針(仮) in 日本語

- GoogleTestを導入してテストを入念に行う
	- printデバッグに可能な限り頼らない
	- CMakeに導入手順まで記述してコマンド一発でできるようにする
	- 文字列に関してはテストケースを柔軟に増減できるようにする
	- ビルドがなぜかエラーしてどうしようもなかったのでIUTESTを導入、ヘッダオンリーで非常に手軽
- r-indexとsdsl-liteの知識を取り入れて高速に実装する
- 探索中に値を保持する必要があるので、br-indexクラスはr-indexクラスと違って探索初期化メンバ関数、パターン拡張メンバ関数などが必要になる
	- locateなど統一的な操作は別ファイルで実装する
- PLCPはLCP_FMN_RLCSAを参考に実装
	- OとZを表すビットベクトルは長さn,1の数rと期待されるのでsd_vectorで簡潔化する(テンプレート化して他でも使えるようにする sd_vectorのラッパーは既に書いているので使える)

## 現状 in 日本語

(以下解決済み)

	PLCPは正常に機能している LFとLFRも機能している

	問題はlocate機構
	サンプルの取得方法、dの更新方法が何かしら間違っている可能性がある

	locateの始点となるサンプルがそもそも間違っている

	判明した問題点
	サンプルの取り方、runの最後の文字を取ってくる方式だったが、
	これの正当性が担保されていたのはSA[ep]を保証するsimplified toehold lemmaのおかげだった
	それが成り立たない今それは使えないので、BWT[ep]がcでないならばsamples_last,
	そうでないならsamples_firstから取ってくるようにしなければならない
	加えて逆でもできるようにsamples_firstRが必要になる
(以上解決済み)


あとはこのREADMEを整備したりコードを少し綺麗にしたり、locateするにあたっては必要ない変数達を除去した
リファクタリング・高速版(inv_orderとinv_orderRがない)を作ったりする

