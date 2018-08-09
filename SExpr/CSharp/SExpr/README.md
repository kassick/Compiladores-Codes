# C# S-Expressions

This is a demo for a S-Expression parser in C#, using Antlr.

On Linux: the antlr rule has some problems locating Java's executable. Build with:
```sh
xbuild /p:Antlr4JavaExecutable=`which java`
```
