# amx_clan_system

create database and import 'clan_system_cs16.sql' in phpmyadmin from import button 

and

in file :
```
addons\amxmodx\configs\modules.ini
```
edit :
```
;mysql
;sqlite
```

to:

```
mysql
;sqlite
```

and put : cts_clan_system.inc into :
```
addons\amxmodx\scripting\include
```

u can use it by sma scripts by this :
```pawn
#include <cts_clan_system>
```
