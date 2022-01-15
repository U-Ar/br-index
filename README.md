# br-index

## About

This repository implements __bi-directional__ r-index (br-index).

The r-index is a compressed text index which supports count(P) and locate(P).
Its size is O(r) words, whose r is the number of equal-letter runs in BWT of the text.

The r-index was originally proposed in the following papers:
- Gagie, T., Navarro, G., & Prezza, N. (2018). Optimal-time text indexing in BWT-runs bounded space. In Proceedings of the Twenty-Ninth Annual ACM-SIAM Symposium on Discrete Algorithms (pp. 1459-1477). Society for Industrial and Applied Mathematics.
- Gagie, T., Navarro, G., & Prezza, N. (2020). Fully functional suffix trees and optimal text searching in BWT-runs bounded space. Journal of the ACM (JACM), 67(1), 1-54.

The br-index we have proposed is achieved by using the mechanism of the r-index but adding new structures. It allows for bi-directional extension of patterns, _left-extension_ and _right-extension_. Also, you can locate all the occurrences of the current pattern at any step of the search.

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
