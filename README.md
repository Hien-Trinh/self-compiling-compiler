# Self-hosted Compiler

## What is it?

The code that you write everyday essentially gets executed by an executable/binary, which itself was lines of codes prior to becoming an executable. I like to think of it as the chicken and egg dilemma. Google self-hosted compiler or bootstrapping if you want to learn about history.

## Motivation?

I'm pretty sure most compilers do this, so I got interested.

## How to do it?

True to its name, bootstrapping requires a bootstrap, usually an existing programming language (Assembly, C, etc.) or, if you're a gigachad, use punch cards to program a simple compiler. Then, write your compiler, compile it using the bootstrap compiler, take the compiled compiler to compile the compiler, take the new compiled compiler to compile itself, then compare your two binaries to check if your compiler is working correctly. Easy. Look below for more details ;)

## How I did it

### Planning

- Compile to C: I don't plan on writing Assembly. Compiling to C then use GCC to compile to binary is much simpler.
- Bootstrap to Python: I'm good at python.
- New language (based on C): I called it Dav cause I'm bad at naming things.

### Stage 0 Compiler (Python)

- Write a compiler in Python to translate Dav to C.
- Both languages share similar syntax and grammar so even a baby can do this.
- It's best to not overengineer and try to implement unnecessary features.
- Some important feature to include:
  - functions + return
  - if statements
  - while loops
  - array/string
- WRITE ERROR HANDLING, PLEASE

### Stage 1 Compiler (Dav)

- Write a compiler in Dav to translate Dav to C.
- Port over the logic from your Python compiler.
- Write unsafe C
- Regret
- Debug
- Compile using the stage 0 compiler.
- Debug
- Saved by the error handling
- Debug
- Some notes:
  - Instead of OOP objects, use parallel arrays. Imagine a big table where each column is a property and each row is an object.
  - No hashmap so just write 100 if statements. Technically constant time since the number of checks is fixed.
  - Be careful of ANYTHING that uses pointers. I had to refactor 1000+ lines because of how my string concatenation work.
  - Dav can handle some type inference, not as dynamic as Python but better than nothing.
  - Expression of the same level is handled right-to-left, since I'm too lazy to fix it. It shouldn't break anything except for edge cases like (a == b == c) -> (a == (b == c)) instead of ((a == b) == c).


### Generate the first Dav compiled

```{shell}
python3 python/stage0_compiler.py stage1_compiler.dav stage1_compiler.c && gcc stage1_compiler.c -o stage1_compiler && ./stage1_compiler stage1_compiler.dav stage1a_compiler.c
```

### Generate the second Dav compiled
```{shell}
gcc stage1a_compiler.c -o stage1a_compiler && ./stage1a_compiler stage1_compiler.dav stage1b_compiler.c
```

### Check diff
```{shell}
diff stage1a_compiler.c stage1b_compiler.c
```

If that returns an empty string, our Dav compiler is working as intended. Yay!