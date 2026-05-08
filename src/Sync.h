#pragma once

#include "sdk/SexySDK.hpp"

template<typename T>
bool WriteBufferFrom(Sexy::Buffer* buffer, T* obj) {
    if (!obj) return false;

    Sexy::DataSync* sync = new Sexy::DataSync();
    Sexy::DataWriter* writer = sync->StartWriteMemory(0x20);
    obj->SyncState(sync);

    uchar* dataPtr = *reinterpret_cast<uchar**>(reinterpret_cast<char*>(writer) + 0x8);
    unsigned long dataLen = *reinterpret_cast<unsigned long*>(reinterpret_cast<char*>(writer) + 0xc);

    buffer->WriteBytes(dataPtr, dataLen);
    return true;
}

template<typename T>
bool WriteBufferTo(Sexy::Buffer* buffer, T* obj) {
    if (!obj) return false;

    Sexy::DataSync* sync = new Sexy::DataSync();

    int headerSize = 4; 
    
    const uchar* dataStart = buffer->GetDataPtr() + headerSize;
    int remainingLen = buffer->GetDataLen() - headerSize;

    if (remainingLen <= 0) return false;

    sync->StartReadMemory(dataStart, remainingLen, false);

    try {
        obj->SyncState(sync);
    } catch (...) {
        std::printf("Exception caught during SyncState! Data might be malformed.\n");
        return false;
    }
    return true;
}