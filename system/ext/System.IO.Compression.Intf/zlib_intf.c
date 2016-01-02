#ifdef _WIN32
    #if defined(__MINGW64__)
        #include "zlib/windows/x64/zlib.h"
    #else
        #include "zlib/windows/x86/zlib.h"
    #endif
#endif
#include <stdlib.h>

#define COMPRESS    0
#define DECOMPRESS  1

int zlib_init(int mode, int level, void** handle)
{
    int ret = Z_OK;
    if (!handle)
    {
        ret = Z_MEM_ERROR;
    }
    else
    {
        z_stream* strm = (z_stream*)malloc(sizeof(z_stream));
        switch (mode)
        {
            case COMPRESS:
            {
                strm->zalloc = Z_NULL;
                strm->zfree = Z_NULL;
                strm->opaque = Z_NULL;
                ret = deflateInit(strm, level);                
                break;
            }
            case DECOMPRESS:
            {
                strm->zalloc = Z_NULL;
                strm->zfree = Z_NULL;
                strm->opaque = Z_NULL;
                strm->avail_in = 0;
                strm->next_in = Z_NULL;
                ret = inflateInit(strm);
                break;
            }
        }
        *handle = strm;
    }
    return ret;
}

void zlib_done(int mode, void* handle)
{
    z_stream* strm = (z_stream*)handle;
    switch (mode)
    {
        case COMPRESS:
        {
            deflateEnd(strm);
            break;
        }
        case DECOMPRESS:
        {
            inflateEnd(strm);
            break;
        }
    }    
    free(strm);
}

void zlib_set_input(void* inChunk, int inAvail, void* handle)
{
    z_stream* strm = (z_stream*)handle;
    strm->next_in = inChunk;
    strm->avail_in = inAvail;
}

int zlib_deflate(void* outChunk, int outChunkSize, int* have, int* outAvail, void* handle, int flush)
{
    z_stream* strm = (z_stream*)handle;
    strm->next_out = outChunk;
    strm->avail_out = outChunkSize;
    int ret = deflate(strm, flush);
    *have = outChunkSize - strm->avail_out;
    *outAvail = strm->avail_out;
    return ret;
}

int zlib_inflate(void* outChunk, int outChunkSize, int* have, int* outAvail, int* inAvail, void* handle)
{
    z_stream* strm = (z_stream*)handle;
    strm->next_out = outChunk;
    strm->avail_out = outChunkSize;
    int ret = inflate(strm, Z_NO_FLUSH);
    *have = outChunkSize - strm->avail_out;
    *outAvail = strm->avail_out;
    *inAvail = strm->avail_in;
    return ret;
}
