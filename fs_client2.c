#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include <time.h>


int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Need: <ip1> <port1> <ip2> <port2> \n");
        exit(1);
    }
    const char *ip = argv[1];
    const int port = atoi(argv[2]);

    const char *ip2 = argv[3];
    const int port2 = atoi(argv[4]);
    if (fsMount(ip, port, "foo") < 0) {
        perror("fsMount"); exit(1);
    }

    if (fsMount(ip, port, "foo") >= 0) {
        printf("ERROR: Should not be able to mount the same folder twice\n");
        exit(1);
    }

    if (fsMount(ip2, port2, "foo2") < 0) {
        perror("fsMount"); exit(1);
    }

    int fd;
    if ((fd = fsOpen("foo/file1.txt", 0)) < 0) {
        perror("fsOpen"); exit(1);
    }

    int fd2;
    if ((fd2 = fsOpen("foo2/file2.txt", 0)) < 0) {
        perror("fsOpen"); exit(1);
    }

    if (fsOpen("foo/file2.txt", 0) >= 0) {
        printf("ERROR: There should be no file2.txt in server#1\n");
        exit(1);
    }

    if (fsOpen("foo2/file1.txt", 0) >= 0) {
        printf("ERROR: There should be no file1.txt in server#2\n");
        exit(1);
    }

    if (fsOpen("foo3/file1.txt", 0) >= 0) {
        printf("ERROR: There is no mounted folder called foo3\n");
        exit(1);
    }

    char buf[3001];
    int bytesread;
    int i = 0;
    bytesread = fsRead(fd, (void *)buf, 3000);
    *((char *) buf + bytesread) = '\0';
	printf("the bytes tht were read said : %s\n", buf);
    if (strncmp(buf, "Hello World1", 12) != 0) {
        printf("ERROR: Read on file1.txt should return 'Hello World1' \n");
        exit(1);
    }

    bytesread = fsRead(fd2, (void *)buf, 3000);
    *((char *) buf + bytesread) = '\0';
    if (strncmp(buf, "Hello World2", 12) != 0) {
        printf("ERROR: Read on file2.txt should return 'Hello World2' \n");
        exit(1);
    }

    while ((bytesread = fsRead(fd2, (void *)buf, 3000)) > 0) {
        *((char *) buf + bytesread) = '\0';
        printf("%s", (char *) buf);
        i += 1;
    }
    printf("\n");

    if (fsRemove("foo/file1.txt") >= 0) {
        printf("ERROR: Should not be able to delete file1.txt while its open\n"); exit(1);
    }

    if (fsRemove("foo") >= 0 || fsRemove("foo2") >= 0) {
        printf("ERROR: Should not be able to delete mounted folder\n"); exit(1);
    }

    if (fsClose(fd) < 0) {
        perror("fsClose"); exit(1);
    }

    if (fsClose(fd2) < 0) {
        perror("fsClose"); exit(1);
    }

    if (fsUnmount("foo") < 0) {
        perror("fsUnmount"); exit(1);
    }

    if (fsUnmount("foo2") < 0) {
        perror("fsUnmount"); exit(1);
    }

    printf("All tests passed!\n");
    return 0;
}
