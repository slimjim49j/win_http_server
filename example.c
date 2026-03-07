#include "http.c"
#pragma comment(lib, "httpapi.lib")

int __cdecl
wmain(int argc, wchar_t *argv[])
{
 http_ctx Ctx = {0};
 void *RequestBuffer = ALLOC_MEM(MinRequestBufferLn);
 if (!RequestBuffer)
 {
  return ERROR_NOT_ENOUGH_MEMORY;
 }

 if (HttpInit(&Ctx, RequestBuffer, MinRequestBufferLn))
 {
  HttpRecv(&Ctx);
 }

 HttpRelease(&Ctx);
 return 0;
}
