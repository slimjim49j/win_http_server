#include "http_server.c"
#include "mime.c"
#include <stdio.h>
#include <assert.h>
#pragma comment(lib, "httpapi.lib")
#pragma comment(lib, "pathcch.lib")

#define AllocMem(cb) HeapAlloc(GetProcessHeap(), 0, (cb))
#define FreeMem(ptr) HeapFree(GetProcessHeap(), 0, (ptr))

static void *
DebugReadFile(LPCWSTR Name, uint32_t *BytesRead)
{
 void *Ret = 0;
 HANDLE FileHandle = CreateFileW(Name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
 if (FileHandle)
 {
  LARGE_INTEGER FileSize;
  if (GetFileSizeEx(FileHandle, &FileSize))
  {
   assert(FileSize.QuadPart < 0xffffffff);
   void *FileMemory = VirtualAlloc(0, FileSize.QuadPart, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
   if (FileMemory)
   {
    if (ReadFile(FileHandle, FileMemory, (uint32_t)FileSize.QuadPart, BytesRead, 0) && *BytesRead == (uint32_t)FileSize.QuadPart)
    {
     Ret = FileMemory;
    }
    else
    {
     VirtualFree(FileMemory, 0, MEM_RELEASE);
     FileMemory = 0;
     *BytesRead = 0;
    }
   }
  }
  CloseHandle(FileHandle);
 }

 return Ret;
}


static wchar_t *
GetInputDir(int Argc, wchar_t *Argv[])
{
 wchar_t *InputDir;
 if (Argc == 2)
 {
  InputDir = Argv[1];
 }
 if (Argc < 2)
 {
  InputDir = L".";
 }
 else if (Argc > 2)
 {
  printf("Too many args.");
  InputDir = 0;
 }
 return InputDir;
}

int __cdecl
wmain(int Argc, wchar_t *Argv[])
{
 http_ctx Ctx = {0};
 void *RequestBuffer = AllocMem(MinRequestBufferLn);
 if (!RequestBuffer)
 {
  return ERROR_NOT_ENOUGH_MEMORY;
 }

 wchar_t *InputDir = GetInputDir(Argc, Argv);
 if (!InputDir) 
 {
  return ERROR_INVALID_PARAMETER;
 }
 http_fpath BaseDir = HttpResolveFpathFromWStr(InputDir);
 if (!BaseDir.Ln)
 {
  return ERROR_INVALID_PARAMETER;
 }

 if (HttpInit(&Ctx, RequestBuffer, MinRequestBufferLn, 3000))
 {
  printf("Running at: %S\n", Ctx.Uri);
  for (;;)
  {
   if (HttpRecv(&Ctx))
   {
    http_fpath FilePath = {0};
    HttpResolveReqFpath(&Ctx, &BaseDir, &FilePath);

    if (FilePath.Ln)
    {
     uint32_t BytesRead = 0;
     char *Body = DebugReadFile(FilePath.Path, &BytesRead);
     if (Body)
     {
      char *MimeType = MimeLookupPathW(FilePath.Path, FilePath.Ln + 1);
      HttpRespond(&Ctx, 200, MimeType, Body, BytesRead);
      VirtualFree(Body, 0, MEM_RELEASE);
     }
     else
     {
      Body = "Not Found";
      HttpRespond(&Ctx, 404, "text/html", Body, (uint32_t)strlen(Body));
     }
    }
    else
    {
     char *Body = "Bad Request";
     HttpRespond(&Ctx, 400, "text/html", Body, (uint32_t)strlen(Body));
    }
   }
  }
 }

 HttpRelease(&Ctx);

 return 0;
}
