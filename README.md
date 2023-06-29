# wdrec

A world replacer (text expander) program written in pure c. Very much inspired by [espanso](https://github.com/espanso/espanso).

# Features
- Currently supports windows only (linux support planned)
- No dependencies (Uses only system headers)
- Fast and lightweight
- Command line interface
- Http Rest Api

# Installation

Simply download the program from releases page or build it yourself.

# Building

```
cmake -S . -B build
cmake --build build/
```

# Usage

## Starting
To start the wdrec app run,
```
wdrec --port {port}
```

## Adding a Word

And to add a word to replace do,
```
wdrec add {word} {replace}
```

For example if we want to replace 'hello' with 'world' each time we type 'hello' we do,
```
wdrec add hello world
```

Now if we type 'hello' it will get replaced with 'word'. If we type backspace then we will get the original 'hello' back.

## Removing a Word
And to remove a existing word do,
```
wdrec remove {word}
```

For example if we want to remove the 'hello' replace we do,
```
wdrec add hello
```

## Closing the Server
Do the following to close the server
```
wdrec close
```

## Other Info

See [info.md](/docs/info.md) for more info.

