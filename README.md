# br-index

## About

Bidirectional version of r-index.

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
