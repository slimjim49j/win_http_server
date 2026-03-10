@echo off

cl /W3 /Z7 /Od /nologo example.c /link /INCREMENTAL:NO /DEBUG:NONE
cl /W3 /Z7 /Od /nologo mime_test.c /link /INCREMENTAL:NO /DEBUG:NONE
del *.obj
