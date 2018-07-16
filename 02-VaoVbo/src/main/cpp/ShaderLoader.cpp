//
// Created by luocj on 18-7-16.
//

#include "ShaderLoader.h"

unsigned char *readDataFromFile(char *path) {
    FILE *fp;
    unsigned char *content = NULL;

    int count = 0;

    if (path != NULL) {
        fp = fopen(path, "rb");

        if (fp != NULL) {

            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);

            if (count > 0) {
                content = (unsigned char *) malloc(sizeof(unsigned char) * (count + 1));
                count = fread(content, sizeof(unsigned char), count, fp);
                content[count] = '\0';
            }
            fclose(fp);
        }
    }
    return content;

}

char *readShaderFile(const char *path) {


    FILE *fp;
    char *content = NULL;

    int count = 0;

    if (path != NULL) {
        fp = fopen(path, "rt");

        if (fp != NULL) {

            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);

            if (count > 0) {
                content = (char *) malloc(sizeof(char) * (count + 1));
                count = fread(content, sizeof(char), count, fp);
                content[count] = '\0';
            }
            fclose(fp);
        }
    }
    return content;
}

int writeShaderFile(char *path, char *s) {

    FILE *fp;
    int status = 0;

    if (path != NULL) {
        fp = fopen(path, "w");

        if (fp != NULL) {

            if (fwrite(s, sizeof(char), strlen(s), fp) == strlen(s))
                status = 1;
            fclose(fp);
        }
    }
    return (status);
}