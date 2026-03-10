@echo off

cl /W3 /Zi /Od /nologo example.c /link /INCREMENTAL:NO
cl /W3 /Zi /Od /nologo mime_test.c /link /INCREMENTAL:NO
del *.obj
