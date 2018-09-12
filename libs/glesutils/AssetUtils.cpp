//
// Created by luocj on 18-9-11.
//

#ifndef LOG_TAG
#define LOG_TAG "AssetUtils"
#endif

#include <cstring>
#include <cstdlib>

#ifdef ANDROID

#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/asset_manager.h>

typedef AAsset esFile;
#else
typedef FILE esFile;
#endif

#include <androidnative/MyLog.h>
#include <utils/AssetUtils.h>

typedef struct
#ifdef __APPLE__
    __attribute__ ( ( packed ) )
#endif
#ifdef ANDROID
    __attribute__ ( ( packed ) )
#endif
{
    unsigned char IdSize;
    unsigned char MapType;
    unsigned char ImageType;
    unsigned short PaletteStart;
    unsigned short PaletteSize;
    unsigned char PaletteEntryDepth;
    unsigned short X;
    unsigned short Y;
    unsigned short Width;
    unsigned short Height;
    unsigned char ColorDepth;
    unsigned char Descriptor;
} TGA_HEADER;

static void printTGAHeader(TGA_HEADER Header) {
    MyLOGE("TGA HEADER IdSize=%u MapType=%u ImageType=%u PaletteStart=%u PaletteSize=%u"
                   "PaletteEntryDepth=%u X=%u Y=%u Width=%u, Height=%u, ColorDepth=%u, Descriptor=%u",
           Header.IdSize, Header.MapType, Header.ImageType, Header.PaletteStart, Header.PaletteSize,
           Header.PaletteEntryDepth, Header.X, Header.Y, Header.Width, Header.Height, Header.ColorDepth, Header.Descriptor);
}

static esFile *esFileOpen(void *ioContext, const char *fileName) {
    esFile *pFile = NULL;

#ifdef ANDROID

    if (ioContext != NULL) {
        AAssetManager *assetManager = (AAssetManager *) ioContext;
        pFile = AAssetManager_open(assetManager, fileName, AASSET_MODE_BUFFER);
    }

#else
#ifdef __APPLE__
    // iOS: Remap the filename to a path that can be opened from the bundle.
    fileName = GetBundleFileName ( fileName );
#endif

    pFile = fopen ( fileName, "rb" );
#endif

    return pFile;
}

///
// esFileRead()
//
//    Wrapper for platform specific File close
//
static void esFileClose(esFile *pFile) {
    if (pFile != NULL) {
#ifdef ANDROID
        AAsset_close(pFile);
#else
        fclose ( pFile );
      pFile = NULL;
#endif
    }
}

///
// esFileRead()
//
//    Wrapper for platform specific File read
//
static int esFileRead(esFile *pFile, int bytesToRead, void *buffer) {
    int bytesRead = 0;

    if (pFile == NULL) {
        MyLOGE ("esFileRead FAILED pFile null\n");
        return bytesRead;
    }

#ifdef ANDROID
    bytesRead = AAsset_read(pFile, buffer, bytesToRead);
    MyLOGE ("esFileRead AAsset_read pFile %d %lu %lu\n", bytesRead, sizeof(unsigned short), sizeof(unsigned char));
#else
    bytesRead = fread ( buffer, bytesToRead, 1, pFile );
#endif

    return bytesRead;
}

char *esLoadTGA(void *ioContext, const char *fileName, int *width, int *height) {
    char *buffer;
    esFile *fp;
    TGA_HEADER Header;
    int bytesRead;

    // Open the file for reading
    fp = esFileOpen(ioContext, fileName);

    if (fp == NULL) {
        // Log error as 'error in opening the input file from apk'
        MyLOGE ("esLoadTGA FAILED to load : { %s }\n", fileName);
        return NULL;
    }

    bytesRead = esFileRead(fp, sizeof(TGA_HEADER), &Header);

    *width = Header.Width;
    *height = Header.Height;

    MyLOGE ("esLoadTGA { %s } size %dx%d depth %d \n", fileName, Header.Width, Header.Height,
            Header.ColorDepth);

    printTGAHeader(Header);

    if (Header.ColorDepth == 8 ||
        Header.ColorDepth == 24 || Header.ColorDepth == 32) {
        int bytesToRead = sizeof(char) * (*width) * (*height) * Header.ColorDepth / 8;

        // Allocate the image data buffer
        buffer = (char *) malloc(bytesToRead);

        if (buffer) {
            bytesRead = esFileRead(fp, bytesToRead, buffer);
            esFileClose(fp);

            return (buffer);
        }
    } else {
        MyLOGE ("esLoadTGA FAILED due to color { %s } %dx%d\n", fileName, Header.Width,
                Header.Height);
    }

    return (NULL);
}
