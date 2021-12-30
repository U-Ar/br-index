# br-index

## About

Bidirectional version of r-index.

submoduleを使っているので、
git cloneに--recursiveをつけるのを忘れない

## 完成後消す項目 実装方針

- GoogleTestを導入してテストを入念に行う
	- printデバッグに可能な限り頼らない
	- CMakeに導入手順まで記述してコマンド一発でできるようにする
	- 文字列に関してはテストケースを柔軟に増減できるようにする
- r-indexとsdsl-liteの知識を取り入れて高速に実装する
	- SA-ISなんかは本当に参考になる
- 探索中に値を保持する必要があるので、br-indexクラスはr-indexクラスと違って探索初期化メンバ関数、パターン拡張メンバ関数などが必要になる
	- locateなど統一的な操作は別ファイルで実装する
- PLCPはLCP_FMN_RLCSAを参考に実装
	- OとZを表すビットベクトルは長さn,1の数rと期待されるのでsd_vectorで簡潔化する(テンプレート化して他でも使えるようにする sd_vectorのラッパーは既に書いているので使える)

## 現状

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

