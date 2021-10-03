#include <napi.h>
#include "PhotonAPI.h"
#include <Windows.h>

#include <tchar.h>
#include <string.h> // memcpy()

static PFInit gs_pfInit;
static PFUninit gs_pfUninit;
static PFConnect gs_pfConnect;
static PFDisconnect gs_pfDisconnect;
static PFSearch gs_pfSearch;
static PFGetThumbnail gs_pfGetThumbnail;
static PFFreeString gs_pfFreeString;
static PFFreeBuffer gs_pfFreeBuffer;

static Napi::Number Init(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  int res = gs_pfInit();
  return Napi::Number::New(env, res);
}

static Napi::Number Uninit(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  gs_pfUninit();
  return Napi::Number::New(env, 0);
}

static Napi::Number Connect(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() != 1 || !info[0].IsString())
  {
      Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  } 

  std::string addrStr = info[0].ToString().Utf8Value();
  void *requester = gs_pfConnect(addrStr.c_str());
  int64_t returnValue = reinterpret_cast<int64_t>(requester);
  return Napi::Number::New(env, returnValue);
}

static Napi::Number Disconnect(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() != 1 || !info[0].IsNumber())
  {
      Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  } 

  int64_t requester_int = info[0].As<Napi::Number>().Int64Value();
  void *requester = reinterpret_cast<void *>(requester_int);
  gs_pfDisconnect(requester);
  return Napi::Number::New(env, 0);
}

static Napi::String Search(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() != 7 || 
    !info[0].IsNumber() || // requester
    !info[1].IsString() || // imageURL
    !info[2].IsBuffer() || // image data
    !info[3].IsNumber() || // resolutionLevel
    !info[4].IsNumber() || // rotation
    !info[5].IsBoolean() || // mirrored
    !info[6].IsBoolean() // thorough
  )
  {
      Napi::TypeError::New(env, "Invalid argument type").ThrowAsJavaScriptException();
  } 

  int64_t requester_int = info[0].As<Napi::Number>().Int64Value();
  void *requester = reinterpret_cast<void *>(requester_int);
  std::string imageURL = info[1].ToString().Utf8Value();

  // Retrieve the buffer
  Napi::Buffer<unsigned char> buffer = info[2].As<Napi::Buffer<unsigned char>>();
  const unsigned char *data = buffer.Data();
  size_t dataSize = buffer.Length();
  Napi::Number resLevel = info[3].As<Napi::Number>();
  Napi::Number rotation = info[4].As<Napi::Number>();

  StrInfo strInfo;
  int res = gs_pfSearch(requester, imageURL.c_str(), 
    data, dataSize, resLevel.Int32Value(), rotation.Int32Value(), false, false, &strInfo);

  std::string json(strInfo.str);
  gs_pfFreeString(&strInfo);

  return Napi::String::New(env, json);
}

static Napi::Object GetThumbnail(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() != 3 || 
    !info[0].IsNumber() || // requester
    !info[1].IsNumber() || // imageID
    !info[2].IsString() // desired encoding
  )
  {
      Napi::TypeError::New(env, "Invalid argument type").ThrowAsJavaScriptException();
  }

  int64_t requester_int = info[0].As<Napi::Number>().Int64Value();
  void *requester = reinterpret_cast<void *>(requester_int);
  Napi::Number imageID = info[1].As<Napi::Number>();
  std::string encoding = info[2].ToString().Utf8Value();
  StrInfo strInfo;
  BufInfo bufInfo;

  int res = gs_pfGetThumbnail(requester,  imageID.Int32Value(), encoding.c_str(), &strInfo, &bufInfo);
  Napi::Buffer<unsigned char> buffer = Napi::Buffer<unsigned char>::New(env, bufInfo.dataSize);
  memcpy(buffer.Data(), bufInfo.data, bufInfo.dataSize);
  std::string jsonStr = strInfo.str;
  gs_pfFreeBuffer(&bufInfo);
  gs_pfFreeString(&strInfo);

  Napi::Object obj = Napi::Object::New(env);
  obj.Set("data", buffer);
  obj.Set("properties", jsonStr);

  return obj;
}

//====================================

#define GET_CURR_DIRnot

static Napi::Object InitModule(Napi::Env env, Napi::Object exports)
{
#ifdef GET_CURR_DIR
  const unsigned int maxDirLen = 512;
  TCHAR dirNameBuf[maxDirLen];
  DWORD dwRes = GetCurrentDirectory(maxDirLen, dirNameBuf);
#endif  

  HMODULE hModule = ::LoadLibrary("PhotonAPI_zmq.dll");

  if (nullptr == hModule)
  {
#ifdef GET_CURR_DIR
    // Just to find out current working directory
    Napi::TypeError::New(env, dirNameBuf).ThrowAsJavaScriptException();
#else
    Napi::TypeError::New(env, "Could not load library").ThrowAsJavaScriptException();
#endif
  }

  gs_pfInit = (PFInit)::GetProcAddress(hModule, "Init");
  gs_pfUninit = (PFUninit)::GetProcAddress(hModule, "Uninit");
  gs_pfConnect = (PFConnect)::GetProcAddress(hModule, "Connect");
  gs_pfDisconnect = (PFDisconnect)::GetProcAddress(hModule, "Disconnect");
  gs_pfSearch = (PFSearch)::GetProcAddress(hModule, "Search");
  gs_pfGetThumbnail = (PFGetThumbnail)::GetProcAddress(hModule, "GetThumbnail");
  gs_pfFreeString = (PFFreeString)::GetProcAddress(hModule, "FreeString");
  gs_pfFreeBuffer = (PFFreeBuffer)::GetProcAddress(hModule, "FreeBuffer");

  if (nullptr == gs_pfInit || nullptr == gs_pfUninit || nullptr == gs_pfConnect || nullptr == gs_pfDisconnect || 
    nullptr == gs_pfSearch || nullptr == gs_pfGetThumbnail || nullptr == gs_pfFreeString || nullptr == gs_pfFreeBuffer)
  {
    ::FreeLibrary(hModule);
    Napi::TypeError::New(env, "Could not get addresses of all functions").ThrowAsJavaScriptException();
  }

  exports.Set(Napi::String::New(env, "Init"), Napi::Function::New(env, Init));
  exports.Set(Napi::String::New(env, "Uninit"), Napi::Function::New(env, Uninit));
  exports.Set(Napi::String::New(env, "Connect"), Napi::Function::New(env, Connect));
  exports.Set(Napi::String::New(env, "Disconnect"), Napi::Function::New(env, Disconnect));
  exports.Set(Napi::String::New(env, "Search"), Napi::Function::New(env, Search));
  exports.Set(Napi::String::New(env, "GetThumbnail"), Napi::Function::New(env, GetThumbnail));
              
  return exports;
}

NODE_API_MODULE(addon, InitModule)
