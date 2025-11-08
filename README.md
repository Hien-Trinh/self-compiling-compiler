# self-compiling-compiler

To compile compiler.dav to C (not implemented)
```{shell}
python3 python/compiler.py compiler.dav compiler.c
```

To compile type_checking.dav to C
```{shell}
python3 python/compiler.py playground/type_checking.dav playground/type_checking.c
```

To compile type_checking to bin
```{shell}
gcc playground/type_checking.c -o playground/type_checking
```

Execute binary
```{shell}
./playground/type_checking
```