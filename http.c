#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <http.h>
#include <stdio.h>
#include <stdint.h>

#define INITIALIZE_HTTP_RESPONSE(resp, status, reason) \
 do                                                    \
 {                                                     \
  RtlZeroMemory((resp), sizeof(*(resp)));              \
  (resp)->StatusCode = (status);                       \
  (resp)->pReason = (reason);                          \
  (resp)->ReasonLength = (USHORT)strlen(reason);       \
 } while (0)

#define ADD_KNOWN_HEADER(Response, HeaderId, RawValue)         \
 do                                                            \
 {                                                             \
  (Response).Headers.KnownHeaders[(HeaderId)].pRawValue =      \
      (RawValue);                                              \
  (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength = \
      (USHORT)strlen(RawValue);                                \
 } while (0)

#define ALLOC_MEM(cb) HeapAlloc(GetProcessHeap(), 0, (cb))
#define FREE_MEM(ptr) HeapFree(GetProcessHeap(), 0, (ptr))

typedef struct http_ctx http_ctx;
struct http_ctx
{
 HANDLE ReqQueue;
 wchar_t *Uri;
 void *RequestBuffer;
 uint32_t RequestBufferLn;
};

enum { MinRequestBufferLn = sizeof(HTTP_REQUEST) + 2048 };

static uint32_t
HttpRespond(
    HANDLE ReqQueue,
    PHTTP_REQUEST Req,
    uint16_t StatusCode,
    char *Reason,
    char *EntityString)
{
 HTTP_RESPONSE response;
 HTTP_DATA_CHUNK dataChunk;
 DWORD result;
 DWORD bytesSent;

 INITIALIZE_HTTP_RESPONSE(&response, StatusCode, Reason);
 ADD_KNOWN_HEADER(response, HttpHeaderContentType, "text/html");

 if (EntityString)
 {
  dataChunk.DataChunkType = HttpDataChunkFromMemory;
  dataChunk.FromMemory.pBuffer = EntityString;
  dataChunk.FromMemory.BufferLength =
      (ULONG)strlen(EntityString);

  response.EntityChunkCount = 1;
  response.pEntityChunks = &dataChunk;
 }

 // Because the entity body is sent in one call, it is not
 // required to specify the Content-Length.
 result = HttpSendHttpResponse(
     ReqQueue,           // ReqQueueHandle
     Req->RequestId, // Request ID
     0,                   // Flags
     &response,           // HTTP response
     NULL,                // pReserved1
     &bytesSent,          // bytes sent  (OPTIONAL)
     NULL,                // pReserved2  (must be NULL)
     0,                   // Reserved3   (must be 0)
     NULL,                // LPOVERLAPPED(OPTIONAL)
     NULL                 // pReserved4  (must be NULL)
 );

 if (result != NO_ERROR)
 {
  wprintf(L"HttpSendHttpResponse failed with %lu \n", result);
 }

 return result;
}

static uint32_t
HttpInit(http_ctx *Ctx, void *RequestBuffer, uint32_t RequestBufferLn)
{
 if (!RequestBuffer || RequestBufferLn < 0)
 {
  return 0;
 }
 Ctx->Uri = L"http://127.0.0.1:3000/";
 Ctx->RequestBuffer = RequestBuffer;
 Ctx->RequestBufferLn = RequestBufferLn;
 ULONG RetCode;
 int32_t UrlAdded = 0;
 HTTPAPI_VERSION HttpApiVersion = HTTPAPI_VERSION_1;

 RetCode = HttpInitialize(HttpApiVersion, HTTP_INITIALIZE_SERVER, NULL);
 if (RetCode != NO_ERROR)
 {
  wprintf(L"HttpInitialize failed with %lu \n", RetCode);
  return 0;
 }

 RetCode = HttpCreateHttpHandle(&Ctx->ReqQueue, 0);
 if (RetCode != NO_ERROR)
 {
  wprintf(L"HttpCreateHttpHandle failed with %lu \n", RetCode);
  return 0;
 }

 RetCode = HttpAddUrl(Ctx->ReqQueue, Ctx->Uri, NULL);

 if (RetCode != NO_ERROR)
 {
  wprintf(L"HttpAddUrl failed with %lu \n", RetCode);
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
 uint32_t Ret = 0;
 HANDLE ReqQueue = Ctx->ReqQueue;
 HTTP_REQUEST_ID RequestId;
 DWORD BytesRead;
 PHTTP_REQUEST Req = (PHTTP_REQUEST)Ctx->RequestBuffer;

 // Wait for a new request. This is indicated by a NULL request ID.
 HTTP_SET_NULL_ID(&RequestId);
 // RtlZeroMemory(Req, Ctx->RequestBufferLn);

 if (!HttpReceiveHttpRequest(ReqQueue, RequestId, HTTP_RECEIVE_REQUEST_FLAG_FLUSH_BODY, Req, Ctx->RequestBufferLn, &BytesRead, NULL))
 {
  Ret = 1;
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
