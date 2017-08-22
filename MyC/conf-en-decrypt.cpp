#include <stdio.h>
#include <stdlib.h>
#include "conf_ssl.h"

// compile command line: g++ -o conf-en-decrypt conf-en-decrypt.cpp conf_ssl.cpp -lssl -lcrypto -std=c++11

int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("Usage: conf-en-decrypt mode <original file> <output file>\n");
        printf("       mode: 0 == decrypt, 1 == encrypt\n");
        return -1;
    }

    int mode = atoi(argv[1]);
    char* ifname = argv[2];
    char* ofname = argv[3];

    CStreamConfSSL ssl(ifname, mode ? CStreamConfSSL::modeEncrypt : CStreamConfSSL::modeDecrypt);

    if (ssl.is_open()) {
        std::ofstream of(ofname);

        if (of.is_open()) {
            char line[1024] = { 0 };
            while (true)
            {
                int outl = ssl.readsome(line, 1024);
                if (outl <= 0) break;
                of.write(line, outl);
            }
            printf("save %s success\n", ofname);
        }
        else {
            printf("open file %s failed!\n", ofname);
        }
    }
    else {
        printf("open file %s failed!\n", ifname);
    }
    return 0;
}
