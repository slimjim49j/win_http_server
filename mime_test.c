#pragma once
#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mime.c"

#pragma comment(lib, "pathcch.lib")

static uint32_t
MimeLookupExtnTest()
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

  char *Res = MimeLookupExtnW(ExtW);
  if (!Res)
  {
   Ret = 0;
  }
  
  if (!Res)
  {
   printf("MimeLookupExtnW failed for: %S\n", ExtW);
  }
 }

 if (Ret)
 {
  printf("MimeLookupExtnW test passed!\n");
 }
 else
 {
  printf("MimeLookupExtnW test failed.\n");
 }

 return Ret;
}

static uint32_t
MimeLookupExtnFailTest()
{
 char *Res = MimeLookupExtnW(L".jpegasdfasdfasdf");
 if (Res)
 {
  printf("MimeLookupExtnW fail test failed.\n");
 }
 else
 {
  printf("MimeLookupExtnW fail test passed!\n");
 }
 return !Res;
}

static uint32_t
MimeLookupPathTest()
{
 wchar_t *Path = L".\\mime.jpeg";
 char *Res = MimeLookupPathW(Path, wcslen(Path) + 1);
 if (Res && !strcmp(Res, "image/jpeg"))
 {
  printf("MimeLookupPathW basic test passed!\n");
  return 1;
 }
 else
 {
  printf("MimeLookupPathW basic test failed.\n");
  return 0;
 }
}

static uint32_t
MimeLookupPathFailTest()
{
 wchar_t *Path = L".\\asdf";
 char *Res = MimeLookupPathW(Path, wcslen(Path) + 1);
 if (Res && !strcmp(Res, "application/octet-stream"))
 {
  printf("MimeLookupPathW fail test passed!\n");
  return 1;
 }
 else {
  printf("MimeLookupPathW fail test failed.\n");
  return 0;
 }
}

int wmain(int argc, wchar_t const *argv[])
{
 uint32_t FailCnt = 0;
 uint32_t TestCnt = 4;
 FailCnt += !MimeLookupExtnTest();
 FailCnt += !MimeLookupExtnFailTest();
 FailCnt += !MimeLookupPathTest();
 FailCnt += !MimeLookupPathFailTest();
 printf("%d tests passed, %d tests failed.\n", TestCnt - FailCnt, FailCnt);
 return 0;
}
