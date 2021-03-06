# Midnight Clock

## 概要

かの有名な(?) 世界終末時計(Doomsday Clock) を再現したアナログ時計です。

![midnight-clock](https://user-images.githubusercontent.com/52535984/156348377-d1e485b7-57ea-422f-b450-822eca4b884f.png)

## 使い方

実行すると、「Midnight Clock」「Number Dial」「History」の 3 つのウィンドウが開きます。

- Midnight Clock は、時計盤を表示するウィンドウです。
  時計盤はピザのように四分割されており、針が向いている部分だけ表示されます。
  元ネタと同様、時字は黒い丸になっています。
- Number Dial を Midnight Clock に重ねると、時字が数字になります。
  ウィンドウを拡大・縮小しても大丈夫です。
- History には、世界終末時計の更新・声明があった年が表示されます。
  年をクリックすると、その年の終末時計の時刻を確認することができます。
  人類の歴史を感じてみましょう。

## ビルド方法

このソフトウェアは、32bit 版 Cygwin を対象に作成されました。

- このソフトウェアは、OpenGL、GLUT、glpng を使用しています。導入していない場合は、先に導入してください。
- `gcc -I/usr/include/opengl -o mclock mclock.c -lglpng -lglut32 -lglu32 -lopengl32` でビルドできます。
- ビルド時に GLUT が警告を出すかもしれません。無視しても大丈夫です。
- (Makefile を用意したかったのですが、当方の環境だと make が動きませんでした…)

## その他

- 様々な理由から、文字は全て png 画像にしています。ダサいですが勘弁してください。
- 若気の至りで、コードにかなりクセがあります(今見ると自分でも謎です)。

## 謝辞

glpng の作成者、Ben Wyatt 氏に特別な感謝を表明いたします。
