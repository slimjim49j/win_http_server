#pragma once

#ifndef UNICODE
# define UNICODE
#endif

#ifndef _WIN32_WINNT
# define _WIN32_WINNT 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <http.h>
#include <stdint.h>
#include <stdlib.h>

#define HttpArrLn(Arr) (sizeof(Arr) / sizeof(Arr[0]))

#define HttpInitResponse(resp, status, reason)         \
 do                                                    \
 {                                                     \
  RtlZeroMemory((resp), sizeof(*(resp)));              \
  (resp)->StatusCode = (status);                       \
  (resp)->pReason = (reason);                          \
  (resp)->ReasonLength = (USHORT)strlen(reason);       \
 } while (0)

#define HttpAddHeader(Response, HeaderId, RawValue)            \
 do                                                            \
 {                                                             \
  (Response).Headers.KnownHeaders[(HeaderId)].pRawValue =      \
      (RawValue);                                              \
  (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength = \
      (USHORT)strlen(RawValue);                                \
 } while (0)

typedef struct http_fpath http_fpath;
struct http_fpath
{
 wchar_t Path[MAX_PATH];
 uint32_t Ln;
};

typedef struct http_ctx http_ctx;
struct http_ctx
{
 HANDLE ReqQueue;
 wchar_t Uri[256];
 void *RequestBuffer;
 uint32_t RequestBufferLn;
 HTTP_REQUEST *Req;
};

enum { MinRequestBufferLn = sizeof(HTTP_REQUEST) + 2048 };

static http_fpath
HttpResolveFpathFromWStr(wchar_t *Path)
{
 http_fpath Ret = {0};
 Ret.Ln = GetFullPathNameW(Path, MAX_PATH, Ret.Path, 0);
 return Ret;
}

static void
HttpResolveReqFpath(http_ctx *Ctx, http_fpath *BaseDir, http_fpath *Out)
{
 wchar_t CatStr[MAX_PATH] = {0};
 wcscat_s(CatStr, MAX_PATH, BaseDir->Path);
 wcsncat_s(CatStr, MAX_PATH, Ctx->Req->CookedUrl.pAbsPath, Ctx->Req->CookedUrl.AbsPathLength);

 Out->Ln = GetFullPathNameW(CatStr, MAX_PATH, Out->Path, 0);
 if (Out->Ln < BaseDir->Ln)
 {
  *Out = (http_fpath){0};
 }
}

static uint32_t
HttpRespond(http_ctx *Ctx, uint16_t StatusCode, char *ContentType, char *Body, uint32_t BodyLn)
{
 uint32_t Ret = 1;
 HTTP_RESPONSE Response;
 HTTP_DATA_CHUNK DataChunk;
 DWORD Result;
 DWORD bytesSent;

 HttpInitResponse(&Response, StatusCode, "");
 HttpAddHeader(Response, HttpHeaderContentType, ContentType);

 if (Body)
 {
  DataChunk.DataChunkType = HttpDataChunkFromMemory;
  DataChunk.FromMemory.pBuffer = Body;
  DataChunk.FromMemory.BufferLength = BodyLn;

  Response.EntityChunkCount = 1;
  Response.pEntityChunks = &DataChunk;
 }

 // Because the entity body is sent in one call, it is not
 // required to specify the Content-Length.
 Result = HttpSendHttpResponse(
  Ctx->ReqQueue,       // ReqQueueHandle
  Ctx->Req->RequestId, // Request ID
  0,                   // Flags
  &Response,           // HTTP response
  NULL,                // pReserved1
  &bytesSent,          // bytes sent  (OPTIONAL)
  NULL,                // pReserved2  (must be NULL)
  0,                   // Reserved3   (must be 0)
  NULL,                // LPOVERLAPPED(OPTIONAL)
  NULL                 // pReserved4  (must be NULL)
 );

 if (Result != NO_ERROR)
 {
  // wprintf(L"HttpSendHttpResponse failed with %lu \n", Result);
  Ret = 0;
 }

 return Ret;
}

static uint32_t
HttpInit(http_ctx *Ctx, void *RequestBuffer, uint32_t RequestBufferLn, uint32_t Port)
{
 if (!RequestBuffer || RequestBufferLn < 0)
 {
  return 0;
 }

 {
  wchar_t *UriPart = L"http://127.0.0.1:";
  wchar_t PortStr[_MAX_ULTOSTR_BASE10_COUNT] = {0};
  _ultow_s(Port, PortStr, HttpArrLn(PortStr), 10);
  wchar_t *PathPart = L"/";
  memset(Ctx->Uri, 0, HttpArrLn(Ctx->Uri));
  wcsncat_s(Ctx->Uri, HttpArrLn(Ctx->Uri), UriPart, wcslen(UriPart));
  wcsncat_s(Ctx->Uri, HttpArrLn(Ctx->Uri), PortStr, wcslen(PortStr));
  wcsncat_s(Ctx->Uri, HttpArrLn(Ctx->Uri), PathPart, wcslen(PathPart));
 }
 Ctx->RequestBuffer = RequestBuffer;
 Ctx->RequestBufferLn = RequestBufferLn;
 ULONG RetCode;
 int32_t UrlAdded = 0;
 HTTPAPI_VERSION HttpApiVersion = HTTPAPI_VERSION_1;

 RetCode = HttpInitialize(HttpApiVersion, HTTP_INITIALIZE_SERVER, NULL);
 if (RetCode != NO_ERROR)
 {
  // wprintf(L"HttpInitialize failed with %lu \n", RetCode);
  return 0;
 }

 RetCode = HttpCreateHttpHandle(&Ctx->ReqQueue, 0);
 if (RetCode != NO_ERROR)
 {
  // wprintf(L"HttpCreateHttpHandle failed with %lu \n", RetCode);
  return 0;
 }

 RetCode = HttpAddUrl(Ctx->ReqQueue, Ctx->Uri, NULL);

 if (RetCode != NO_ERROR)
 {
  // wprintf(L"HttpAddUrl failed with %lu \n", RetCode);
  return 0;
 }
 else
 {
  UrlAdded++;
 }

 return 1;
}

static uint32_t
HttpRecv(http_ctx *Ctx)
{
 uint32_t Ret = 1;
 Ctx->Req = (HTTP_REQUEST *)Ctx->RequestBuffer;
 HANDLE ReqQueue = Ctx->ReqQueue;
 HTTP_REQUEST_ID RequestId;
 // Wait for a new request. This is indicated by a NULL request ID.
 HTTP_SET_NULL_ID(&RequestId);
 DWORD BytesRead;

 if (HttpReceiveHttpRequest(ReqQueue, RequestId, HTTP_RECEIVE_REQUEST_FLAG_FLUSH_BODY, Ctx->Req, Ctx->RequestBufferLn, &BytesRead, NULL))
 {
  Ret = 0;
  Ctx->Req = 0;
 }

 return Ret;
}

static void
HttpRelease(http_ctx *Ctx)
{
 HttpRemoveUrl(Ctx->ReqQueue, Ctx->Uri);
 if (Ctx->ReqQueue)
 {
  CloseHandle(Ctx->ReqQueue);
 }
 HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);
}
