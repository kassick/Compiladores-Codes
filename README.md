# Informação

Parser de S-expressões desenvolvido em C++.

Versões:

- Naive [Código](./SExpr/Naive/sparser_naive.cpp): Implementação sem técnica nenhuma, percorrendo um stream de entrada.

- LL(1) (sparser_2.cpp) [Código](./SExpr/LL1/sparser_naive.cpp): Parser Descendente Recursivo com 1 token de lookahead. Não usa tabela.

- Naive Tokens [Código](./SExpr/Naive-Tokens/sparser_naive_tokens.cpp): Implementação sem técnica, porém utilizando um *lexer* (ANTLR) para obter uma lista de tokens

- ANTLR ALL(*) [Código](./SExpr/Antlr/sparser_antlr.cpp): Implementação usando uma gramática do ANTLR, com ações embutidas na gramática

- ANTLR ALL(*) Listener [Código](./SExpr/AntlrListener/sparser_antlr_listener.cpp): Implementação usando uma gramática ANTLR, com código em um Listener.

*Extra*: Projeto em C#, sem código ainda: [Código](./CSharp)

# Versão Javascript

O código em C/C++ pode ser compilado para *asm.js* e executado no navegador. Os arquivos org incluem o arquivo javascript correto e executam a função de parse e apresentam o resultado em um textarea.

Deve-se utilizar o [EMScripten](https://github.com/juj/emsdk) para compilar o código C++ -> Javascript.

A compilação foi testada com os seguintes componentes da SDK:

- sdk-tag-1.37.18-64bit (compiled from source)
- emscripten-tag-1.37.18-64bit (compiled from source)
- node-4.1.1-64bit (downloaded)

Em um diretório de build (E.g. ./JS):

- Carrege o environment do emsdk:

```sh
source ../emsdk-portable/emsdk_env.sh
```

- Execute o `cmake` com o `emconfigure`. O emconfigure seta o compilador e outros detalhes para o cmake

```sh
emconfigure cmake ..
```

- Execute `make` com o `emmake`. O emmake seta algumas variáveis de ambiente para o make funcionar.

```sh
emmake make
```

- Ao fim da compilação, ao invés de executáveis binários, deve ter sido gerados arquivos `.js`. Normalmente, esses arquivos poderiam ser executados como um programa para o node. Porém, eu instruo a compilação a exportar um símbolo `parse_string_c`. Isso faz com que a compilação gere um javascript para importação.

```sh
if(${CMAKE_CXX_COMPILER} MATCHES "em\\+\\+")
  set(CMAKE_EXE_LINKER_FLAGS "-s EXPORTED_FUNCTIONS=\"['_parse_string_c']\"")
endif()
```

  Esse javascript pode ser utilizado dentro de um HTML, usando a função `ccall`:

```html
<script type="text/javascript" src="./sparser.js"></script>

<script>
  doParse = function(text) {
      r = ccall('parse_string_c', 'string', ['string'], [text]);
      return r;
  };

  parseSource = function() {
      d_ta = document.getElementById('esource');
      d_res = document.getElementById('result');

      res = doParse(d_ta.value);

      d_res.value = res;
  };
</script>
```

## Versão Antiga da Compilação para WEB

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
