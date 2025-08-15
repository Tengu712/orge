# Coding Conventions

## Naming Conventions

- `UPPER_SNAKE_CASE`
  - マクロ名
  - 列挙子名 (`enum`)
- `UpperCamelCase`
  - 型名
  - 列挙子名 (`enum class`)
- `_lowerCamelCase`
  - privateメンバ変数名
  - privateメンバ関数名
- `lowerCamelCase`
  - 変数名
  - 関数名

## Tabs for Indentation

インデントにはタブ文字を用いる。

## 120 Column

原則1行最大120文字。
タブ文字のせいで一見すると120文字を超えていることもあるが、面倒臭いので無視。

## auto

`auto`が使えるなら使う。

## const

`const`が使えるなら使う。

## No namespace Indentation

`namespace`ブロックはインデントしない。
凄く気持ち悪いけど仕方ない。

## enum class

`enum`の代わりに`enum class`を用いる。

## pragma once

Include guardの代わりに`#pragma once`を書く。

## Namespace Base Directory

名前空間に即してディレクトリ構成を行う。

## No Legacy Code

API部分を除いてC由来のレガシーなコードを書かない。

## Concise Code

コードは負債である。
結果が変わらない限り、コード量は少ない方が良い。
とはいえ、コード量を減らすことだけに注力して、ifを一行にしたり、関数名を省略したりすると、かえって可読性が落ちる。
あくまで、ヒューマンエラーを減らすためにコード量を減らしていることに注意する。

## Global Variables

グローバル変数を禁止しない。
そのインスタンスが単一であるオブジェクトはグローバル変数で管理した方が間違いなく無駄がなくて良いため。
