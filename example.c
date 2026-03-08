#include "http.c"
#pragma comment(lib, "httpapi.lib")

#define AllocMem(cb) HeapAlloc(GetProcessHeap(), 0, (cb))
#define FreeMem(ptr) HeapFree(GetProcessHeap(), 0, (ptr))

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
  for (;;)
  {
   if (HttpRecv(&Ctx))
   {
    http_fpath FilePath = {0};
    HttpResolveReqFpath(&Ctx, &BaseDir, &FilePath);

    if (FilePath.Ln)
    {
     char *Body = "My response";
     HttpRespond(&Ctx, 200, "text/html", Body, strlen(Body));
    }
    else
    {
     char *Body = "Bad Request";
     HttpRespond(&Ctx, 400, "text/html", Body, strlen(Body));
    }
   }
  }
 }

 HttpRelease(&Ctx);
 return 0;
}
