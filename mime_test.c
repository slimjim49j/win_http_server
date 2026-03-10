#pragma once
#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mime.c"

static uint32_t
MimeTest()
{
 uint32_t Ret = 1;
 for (uint32_t I = 0; I < MimeArrLn(MimeTab); ++I)
 {
  char *ExtName = (char *)MimeTab[I][0];
  char *Mime = (char *)MimeTab[I][1];
  char Ext[32] = {0};

  Ext[0] = '.';
  strncat_s(Ext, MimeArrLn(Ext), ExtName, strlen(ExtName));

  wchar_t ExtW[32] = {0};
  mbstowcs_s(0, ExtW, MimeArrLn(ExtW), Ext, strlen(Ext));

  char *Res = MimeLookupW(ExtW);
  if (!Res)
  {
   Ret = 0;
  }
  
  if (!Res)
  {
   printf("Failed for: %S\n", ExtW);
  }
 }

 if (Ret)
 {
  printf("Mime test passed!\n");
 }
 else
 {
  printf("Mime test failed.\n");
 }

 return Ret;
}

static uint32_t
MimeFailTest()
{
 char *Res = MimeLookupW(L".jpegasdfasdfasdf");
 if (Res)
 {
  printf("Mime fail test failed.\n");
 }
 else
 {
  printf("Mime fail test passed!\n");
 }
 return !Res;
}

int wmain(int argc, wchar_t const *argv[])
{
 uint32_t FailCnt = 0;
 uint32_t TestCnt = 2;
 FailCnt += !MimeTest();
 FailCnt += !MimeFailTest();
 printf("%d tests passed, %d tests failed.\n", TestCnt - FailCnt, FailCnt);
 return 0;
}

