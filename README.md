# MAT_reproduce  (WIP!)
Made-to-order spiking neuron model equipped with a multi-timescale adaptive thresholdのC言語版再実装。

電流の計算と電圧の計算をどちらもひとつのコードでやるようにしています。

1. [メルセンヌツイスター](http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/SFMT/index-jp.html#SFMT)をご用意ください。Makefileに書いてあるSFMTDIRの通りの階層構造になるようにフォルダを置いてください。
2. make matでコンパイルします。
3. ./mat (alpha1) (alpha2) (omega) (i_const)というように実行してください。

i_constは電流を定数にするかどうかを決めます。1なら定数、0ならランダムです。


simple_mat.cの方はコメントアウト2行目のコンパイル文で動かしてください。
