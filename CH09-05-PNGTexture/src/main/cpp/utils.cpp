//
// Created by luocj on 2/12/18.
//
#include<stdio.h>
#include <vector>

#include <androidnative/NativeMain.h>

#include "utils.h"

int myHex2Str(const byte *src, byte *dest, int nSrcLen) {
    if (src == nullptr || dest == nullptr || nSrcLen <= 0) {
        return 0;
    }

    int i;
    char tmp[3];

    for (i = 0; i < nSrcLen; i++) {
        // 检查 sprintf 的返回值
        int result = std::sprintf(tmp, "%02X", static_cast<byte>(src[i]));
        if (result != 2) {
            // 处理 sprintf 错误
            ALOGE("sprintf error in myHex2Str at index %d", i);
            return 0;
        }
        // 检查 memcpy 的目标地址是否有效
        if (&dest[i * 2] == nullptr) {
            ALOGE("Invalid destination address in myHex2Str at index %d", i);
            return 0;
        }
        std::memcpy(&dest[i * 2], tmp, 2);
    }
    return i * 2;
}

// 显示字节数据的十六进制字符串
void my_show_byte(const char *str, const byte bytes[], int size) {
    if (str == nullptr || bytes == nullptr || size <= 0) {
        return;
    }

    // 使用 std::vector 替代可变长度数组
    std::vector<byte> hexStr(size * 2 + 1, 0);

    int len = myHex2Str(bytes, hexStr.data(), size);
    if (len > 0) {
        hexStr[size * 2] = '\0';
        ALOGE("png_show_byte %s %d bytes %s", str, len, reinterpret_cast<const char*>(hexStr.data()));
    }
}