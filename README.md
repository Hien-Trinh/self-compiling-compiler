# self-compiling-compiler

Stage0-1-2

```{shell}
python3 python/stage0_compiler.py stage1_compiler.dav stage1_compiler.c && gcc stage1_compiler.c -o stage1_compiler && ./stage1_compiler stage1_compiler.dav stage2_compiler.c
```