#pragma once

#include <unistd.h>
#include <inttypes.h>
#include <util/AudioMode.h>
#include <util/ErrorCode.h>
#include <util/Option.h>
#include "Interface.h"

static void* Create();
static void Destroy(void* ptr);
static mous::ErrorCode Open(void* ptr, const char* url);
static void Close(void* ptr);
static mous::ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count);
static mous::ErrorCode SetUnitIndex(void* ptr, uint64_t index);
static uint32_t GetMaxBytesPerUnit(void* ptr);
static uint64_t GetUnitIndex(void* ptr);
static uint64_t GetUnitCount(void* ptr);
static mous::AudioMode GetAudioMode(void* ptr);
static int32_t GetChannels(void* ptr);
static int32_t GetBitsPerSample(void* ptr);
static int32_t GetSampleRate(void* ptr);
static int32_t GetBitRate(void* ptr);
static uint64_t GetDuration(void* ptr);
static const mous::BaseOption** GetOptions(void* ptr);
static const char** GetSuffixes(void* ptr);

static mous::DecoderInterface decoder_interface {
    Create,
    Destroy,
    Open,
    Close,
    DecodeUnit,
    SetUnitIndex,
    GetMaxBytesPerUnit,
    GetUnitIndex,
    GetUnitCount,
    GetAudioMode,
    GetChannels,
    GetBitsPerSample,
    GetSampleRate,
    GetBitRate,
    GetDuration,
    GetOptions,
    GetSuffixes
};

extern "C" {
    const mous::DecoderInterface* MousGetDecoderInterface() {
        return &decoder_interface;
    }
}