bcc32 -c -tWM -tW -Ixxxxxxxx -Lyyyyyyyy ace.c converter.c
brc32 ace.rc
ilink32 /aa /Tpe /Gn /t /M c0w32 ace.obj converter.obj, ace.exe,,import32 cw32mt,,ace.res