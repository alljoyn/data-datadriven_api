/******************************************************************************
 * Copyright (c) 2013-2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <sys/types.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GuidUtil.h"
#if defined(QCC_OS_DARWIN)
#include <limits.h>
#include <CoreFoundation/CFUUID.h>
#elif defined(_WIN32)
#include <limits.h>
#else
#include <linux/limits.h>
#endif
#ifdef _WIN32
#include <Rpc.h>
#pragma comment(lib, "rpcrt4.lib")
#endif

//static const char DEVICE_ID_FILE_NAME[] = "alljoyn-deviceId.txt";

using namespace datadriven;
using namespace qcc;

GuidUtil* GuidUtil::pGuidUtil = NULL;

GuidUtil* GuidUtil::GetInstance()
{
    if (pGuidUtil == NULL) {
        pGuidUtil = new GuidUtil();
    }
    return pGuidUtil;
}

GuidUtil::GuidUtil()
{
}

GuidUtil::~GuidUtil()
{
}

void GuidUtil::NormalizeString(char* strGUID)
{
    //remove the '-' from the string:
    std::string sGUID(strGUID);
    std::string::size_type nposition = std::string::npos;
    while ((nposition = sGUID.find('-')) != std::string::npos) {
        sGUID.erase(nposition, 1);
    }
    strcpy(strGUID, sGUID.c_str());
}

void GuidUtil::GenerateGUIDUtil(char* strGUID)
{
#ifdef _WIN32
    UUID uuid;
    UuidCreate(&uuid);
    RPC_CSTR str;
    UuidToStringA(&uuid, &str);
    memcpy(strGUID, str, GUID_STRING_MAX_LENGTH + GUID_HYPHEN_MAX_LENGTH);
    strGUID[GUID_STRING_MAX_LENGTH + GUID_HYPHEN_MAX_LENGTH + 1] = 0;
    RpcStringFreeA(&str);
#elif defined(QCC_OS_DARWIN)
    CFUUIDRef cfUUID = CFUUIDCreate(NULL);
    CFStringRef cfUUIDString = CFUUIDCreateString(NULL, cfUUID);
    const int len = CFStringGetLength(cfUUIDString);
    CFStringGetCString(cfUUIDString, strGUID, len + 1, CFStringGetSystemEncoding());
    CFRelease(cfUUID);
    NormalizeString(strGUID);
#else
    std::ifstream ifs("/proc/sys/kernel/random/uuid", std::ifstream::in);
    ifs.getline(strGUID, GUID_STRING_MAX_LENGTH + GUID_HYPHEN_MAX_LENGTH + END_OF_STRING_LENGTH);
    ifs.close();
    NormalizeString(strGUID);
#endif
}

void GuidUtil::GenerateGUID(qcc::String* guid)
{
    if (guid == NULL) {
        return;
    }

    char tempstrGUID[GUID_STRING_MAX_LENGTH + GUID_HYPHEN_MAX_LENGTH + END_OF_STRING_LENGTH];
    GenerateGUIDUtil(tempstrGUID);
    guid->assign(tempstrGUID);
}
