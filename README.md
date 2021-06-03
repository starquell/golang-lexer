# golang-lexer

[Specification](https://docs.google.com/document/d/1hro0bH9SGsFGwS_RUtjoKF7ud-OrgWdmZJKFIEmucUE/edit)

Tested on files in `examples/`

### Requirements

* CMake 3.9+;
* C++20 aware compiler (tested on GCC 10);
* [fmt](https://github.com/fmtlib/fmt) 6.0+

### Build steps

```bash
git clone https://github.com/starquell/golang-lexer.git
cd golang-lexer
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Usage

in `build/bin`:
```bash
./lexer <filename>.go
```
