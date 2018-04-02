# ColosseumGameSerive
Mainly based on the work of [ACPC](http://www.computerpokercompetition.org/).

## 配置

在目录下直接make就好了

## Poker
 具体细节参考protocol.pdf
### Usage
命令行中输入一下
``` 
$ ./dealer_poker matchName holdem.limit.2p.reverse_blinds.game 1000 0 Alice Bob
```

* 第一个是log文件的名称，对应的会在dealer的目录下，生成一个matchName.log的文件，其中记录对局信息

* 第二个参数是游戏的参数，在这个文件中有定义，具体可以打开这个文件看一下
* 对局次数
* 那个0，现在不需要设置，同样写0就好
* 后面两个就是名字了
### 传输字符串
基本格式
``` shell
    MATCHSTATE:player:handId:betting:holeCards|boardCards:action
```
#### limited game
``` shell
 S-> MATCHSTATE:0:0::TdAs|
S-> MATCHSTATE:0:0:r:TdAs|
<-C MATCHSTATE:0:0:r:TdAs|:r
S-> MATCHSTATE:0:0:rr:TdAs|
S-> MATCHSTATE:0:0:rrc/:TdAs|/2c8c3h
<-C MATCHSTATE:0:0:rrc/:TdAs|/2c8c3h:r
S-> MATCHSTATE:0:0:rrc/r:TdAs|/2c8c3h
S-> MATCHSTATE:0:0:rrc/rc/:TdAs|/2c8c3h/9c
<-C MATCHSTATE:0:0:rrc/rc/:TdAs|/2c8c3h/9c:c
S-> MATCHSTATE:0:0:rrc/rc/c:TdAs|/2c8c3h/9c
S-> MATCHSTATE:0:0:rrc/rc/cr:TdAs|/2c8c3h/9c
<-C MATCHSTATE:0:0:rrc/rc/cr:TdAs|/2c8c3h/9c:c
S-> MATCHSTATE:0:0:rrc/rc/crc/:TdAs|/2c8c3h/9c/Kh
<-C MATCHSTATE:0:0:rrc/rc/crc/:TdAs|/2c8c3h/9c/Kh:c
S-> MATCHSTATE:0:0:rrc/rc/crc/c:TdAs|/2c8c3h/9c/Kh
S-> MATCHSTATE:0:0:rrc/rc/crc/cr:TdAs|/2c8c3h/9c/Kh
<-C MATCHSTATE:0:0:rrc/rc/crc/cr:TdAs|/2c8c3h/9c/Kh:c
S-> MATCHSTATE:0:0:rrc/rc/crc/crc:TdAs|8hTc/2c8c3h/9c/Kh
```
#### nolimit game
``` shell
S-> MATCHSTATE:0:30::9s8h|
S-> MATCHSTATE:0:30:c:9s8h|
<-C MATCHSTATE:0:30:c:9s8h|:c
S-> MATCHSTATE:0:30:cc/:9s8h|/8c8d5c
<-C MATCHSTATE:0:30:cc/:9s8h|/8c8d5c:r250
S-> MATCHSTATE:0:30:cc/r250:9s8h|/8c8d5c
S-> MATCHSTATE:0:30:cc/r250c/:9s8h|/8c8d5c/6s
<-C MATCHSTATE:0:30:cc/r250c/:9s8h|/8c8d5c/6s:r500
S-> MATCHSTATE:0:30:cc/r250c/r500:9s8h|/8c8d5c/6s
S-> MATCHSTATE:0:30:cc/r250c/r500c/:9s8h|/8c8d5c/6s/2d
<-C MATCHSTATE:0:30:cc/r250c/r500c/:9s8h|/8c8d5c/6s/2d:r1250
S-> MATCHSTATE:0:30:cc/r250c/r500c/r1250:9s8h|/8c8d5c/6s/2d
S-> MATCHSTATE:0:30:cc/r250c/r500c/r1250c:9s8h|9c6h/8c8d5c/6s/2d
```
## Renju

### 基本规则

* 第一个玩家执黑先行，然后按照局次轮换先手。
* 越过棋盘的范围，在已有棋子上下子，都会导致失败
* 

### Server启动方法

``` 
$ ./dealer_renju matchName 100 Alice Bob
```
* matchName: 在目录下生成matchName.txt作为log文件
* 100：这里是一个对局次数的设置，这个例子中是对局100次
* Alice Bob: 两个玩家的名字

### 传输字符串
``` shell
General State: MATCHSTATE:viewingplayer:currentplayer:currentGames:currentRounds:finishedFlag:col/row
Final State: MATCHSTATE:viewingplayer:currentplayer:currentGames:currentRounds:finishedFlag
```

## TODO

研究一下他们的perl脚本是怎么写的，或者自己重新写一个