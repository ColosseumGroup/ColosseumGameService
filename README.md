# ColosseumGameSerive
Mainly based on the work of [ACPC](http://www.computerpokercompetition.org/).


## Poker
 具体细节参考protocol.pdf
### examples

#### limited game
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

#### unlimited game
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

## Renju
MATCHSTATE:player:numGames:numRounds:finishedFlag:col/row/type
##