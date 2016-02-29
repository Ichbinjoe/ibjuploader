#include <libgen.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <sys/stat.h>
#include "buffer.h"

#


/*
 * Usage: ibj [options] [file/filename (if using pipe)]
 * Options:
 *  n [name] - sets the name to be uploaded by
 *      If using a pipe and still specify a filename, the specified filename argument will be ignored, and this flag used
 */

size_t httpretcallback(char *ptr, size_t size, size_t nmemb, void *buffer) {
    writeBuf(buffer, ptr, size * nmemb);
}

char *shorten(char *key, char *url) {
    void *curl = curl_easy_init();
    if (curl == NULL) {
        printf("error occured while capturing curl handle\n");
        exit(EXIT_FAILURE);
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://ibj.io/shorten");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ibjio-curl/1.0");

    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;

    curl_formadd(&post, &last, CURLFORM_COPYNAME, "url",
                 CURLFORM_COPYCONTENTS, url, CURLFORM_END);

    curl_formadd(&post, &last, CURLFORM_COPYNAME, "key",
                 CURLFORM_COPYCONTENTS, key, CURLFORM_END);

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    void *writebuf = createBuf();

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpretcallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, writebuf);

    CURLcode curlresult = curl_easy_perform(curl);

    if (curlresult != 0) {
        printf("curl: %s", curl_easy_strerror(curlresult));
        return NULL;
    }

    size_t i = lenbuf(writebuf);
    char *result = realloc(extractFromBuf(writebuf), i + 1);
    result[i] = 0;

    free(writebuf);
    curl_easy_cleanup(curl);
    curl_formfree(post);
    return result;
}

char *upload(char *key, void *data, size_t dataLen, char *filename, bool outputMeter) {
    void *curl = curl_easy_init();
    if (curl == NULL) {
        printf("error occured while capturing curl handle\n");
        exit(EXIT_FAILURE);
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://ibj.io/upload");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ibjio-curl/1.0");
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, !outputMeter);
    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;

    curl_formadd(&post, &last, CURLFORM_COPYNAME, "data",
                 CURLFORM_BUFFER, filename,
                 CURLFORM_BUFFERPTR, data,
                 CURLFORM_BUFFERLENGTH, dataLen,
                 CURLFORM_CONTENTSLENGTH, dataLen, CURLFORM_END);

    curl_formadd(&post, &last, CURLFORM_COPYNAME, "key",
                 CURLFORM_COPYCONTENTS, key, CURLFORM_END);

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    void *writebuf = createBuf();

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpretcallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, writebuf);

    CURLcode curlresult = curl_easy_perform(curl);

    if (curlresult != 0) {
        printf("curl: %s\n", curl_easy_strerror(curlresult));
        return NULL;
    }

    size_t i = lenbuf(writebuf);
    char *result = realloc(extractFromBuf(writebuf), i + 1);
    result[i] = 0;
    free(writebuf);
    curl_easy_cleanup(curl);
    curl_formfree(post);
    return result;
}

void printResult(char *result) {
    if (result == NULL) {
        return;
    }
    printf("https://ibj.io/%s\n", result);
}

int main(int argc, char *argv[]) {
    /*
     * We need to determine if we are piping data in or pulling it from
     * an OS file
     */
    FILE *f;
    char *filename = NULL;
    char *mykey = NULL;
    void *data = NULL;
    bool isShorten = false;
    bool useOutputMeter = true;

    size_t dataLen;

    int c;
    while ((c = getopt(argc, argv, "+n:k:u:s")) != -1) {
        switch (c) {
            case 'u':
                filename = (char *) malloc(strlen(optarg) + 1);
                strcpy(filename, optarg);
                isShorten = true;
                break;
            case 'n':
                filename = (char *) malloc(strlen(optarg) + 1);
                strcpy(filename, optarg);
                break;
            case 'k':
                mykey = (char *) malloc(strlen(optarg) + 1);
                strcpy(mykey, optarg);
                break;
            case 's':
                useOutputMeter = false;
                break;
            default:
                printf("Unknown argument %c\n", c);
                exit(EXIT_FAILURE);
        }
    }

    if (mykey == NULL) {
        char *hm = getenv("HOME");
        size_t ln = strlen(hm);
        char keyfilepath[ln + 11];
        strcpy(keyfilepath, hm);
        strcpy(&keyfilepath[ln], "/.ibjiokey");

        int keyfile = open(keyfilepath, O_RDONLY);
        if (keyfile == -1) {
            //No key!
            perror("no key specified and key file retrieval failed");
            return -1;
        } else {
            void *buff = readOffStream(keyfile, false);
            mykey = extractFromBuf(buff);
            free(buff);
            if (mykey[strlen(mykey) - 1] == '\n') {
                mykey[strlen(mykey) - 1] = 0;
            }
        }
    }

    if (isShorten) {
        char *result;
        //Turns out this is a url instead.
        result = shorten(mykey, filename);
        printResult(result);
        return 0;
    }

    char *suppliedFilename = argv[optind];
    void *buff;

    if (isatty(fileno(stdin))) {
        if (optind == 0) { //No more arguments, requires filename
            printf("Missing filename parameter\n");
            return EXIT_FAILURE;
        }

        f = fopen(suppliedFilename, "r");
        if (f == NULL) {
            perror(suppliedFilename);
            return EXIT_FAILURE;
        }

        if (filename == NULL) {
            filename = basename(suppliedFilename);
        }

        struct stat file_stat;
        stat(filename, &file_stat);
        if (!S_ISREG(file_stat.st_mode)) {
            printf("The path does not appear to reflect a file!\n");
            return EXIT_FAILURE;
        }

        buff = readOffFile(f);

    } else {
        //Lets take the fifo and read shit from that
        //Requires filename param
        if (filename == NULL && optind == 0) {
            printf("Missing filename parameter\n");
            return EXIT_FAILURE;
        } else if (filename == NULL) {
            filename = suppliedFilename;
        }
        //Now lets take the data from the stream, and buffer it
        buff = readOffStream(fileno(stdin), false);
    }



    data = extractFromBuf(buff);
    dataLen = lenbuf(buff);

    printResult(upload(mykey, data, dataLen, filename, useOutputMeter));

    free(buff);
    return EXIT_SUCCESS;
}




