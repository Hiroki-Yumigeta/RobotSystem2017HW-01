# RobotSystem2017HW-01
ロボットシステム学 課題1-デバイスドライバ作成
## 概要
    1. echoコマンドでLチカ
    1. catコマンドでスイッチのステータス表示


1. echoコマンドでLチカ
    * GPIO21に以下のように接続(青色高輝度LED)

![](fig1.jpg)

    * 授業と同様
        `echo 1 >> /dev/swled0` でLEDがON
        `echo 0 >> /dev/swled0` でLEDがOFF
    * [動画1: Lチカ](https://www.youtube.com/)
    * [動画1: Lチカ(コンソール画面)](https://www.youtube.com/)


1. catコマンドでスイッチのステータス表示
    * GPIO20に以下のように接続(10kΩでプルダウン)
    * スイッチを押したことがわかるように赤色LEDを物理的に接続
![](fig2.jpg)
        `cat /dev/swled0` でスイッチの状態を表示(ONで1、OFFで0)
    * [動画1: スイッチ認識](https://www.youtube.com/)
    * [動画1: スイッチ認識(コンソール画面)](https://www.youtube.com/)
