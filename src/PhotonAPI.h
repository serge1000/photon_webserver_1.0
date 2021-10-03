// Copyright 1st1 Technologies LLP, 2020-2021
// All rights reserved
#pragma once

extern "C"
{
struct StrInfo
{
    char *str;
    unsigned int length;
};

struct BufInfo
{
    unsigned char *data;
    unsigned int dataSize;
};

#ifdef USE_STATICIC_LIBRARY

int Init();
int Uninit();
void * Connect(const char *address);
void Disconnect(void *requester);

int Search(void *requester, 
    const char *imageURL, const unsigned char *imageData, unsigned int imageDataSize,
    unsigned int resolutionLevel, unsigned short rotation, bool mirrored, bool thorough, StrInfo *pStrInfo);

int GetThumbnail(void *requester, 
    unsigned int imageID, const char *encoding, StrInfo *pStrInfo, BufInfo *pBufInfo);

void FreeString(StrInfo *pStrInfo);
void FreeBuffer(BufInfo *pBufInfo);

#else
typedef int (*PFInit)();
typedef int (*PFUninit)();
typedef void * (*PFConnect)(const char *address);
typedef void (*PFDisconnect)(void *requester);

typedef int (*PFSearch)(void *requester, 
    const char *imageURL, const unsigned char *imageData, unsigned int imageDataSize,
    unsigned int resolutionLevel, unsigned short rotation, bool mirrored, bool thorough, StrInfo *pStrInfo);

typedef int (*PFGetThumbnail)(void *requester, 
    unsigned int imageID, const char *encoding, StrInfo *pStrInfo, BufInfo *pBufInfo);

typedef void (*PFFreeString)(StrInfo *pStrInfo);
typedef void (*PFFreeBuffer)(BufInfo *pBufInfo);
#endif
} // extern "C"