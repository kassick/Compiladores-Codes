# Informação

Parser de S-expressões desenvolvido em C++.

Versões:
- Naive (sparser.cpp)
- LL(1) (sparser_2.cpp)

# Versão Web

Usando emcc:

```sh
emcc -o sparser2.js sparser_2.cpp -s EXPORTED_FUNCTIONS="['_parse_string_c']"
```

Lembrar de fazer:
```sh
emsdk activate sdk-tag-1.37.18-64bit
source emsdk_env.sh
```

Para gerar o console:
```sh
emcc -o sparser_console.html sparser_2.cpp
```
