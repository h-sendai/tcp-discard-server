#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "my_signal.h"
#include "my_socket.h"
#include "get_num.h"
#include "logUtil.h"
#include "set_cpu.h"

int debug = 0;

int print_result(struct timeval start, struct timeval stop, int so_snd_buf, unsigned long long send_bytes)
{
    struct timeval diff;
    double elapse;

    timersub(&stop, &start, &diff);
    fprintfwt(stderr, "server: SO_SNDBUF: %d (final)\n", so_snd_buf);
    elapse = diff.tv_sec + 0.000001*diff.tv_usec;
    fprintfwt(stderr, "server: %.3f MB/s ( %lld bytes %ld.%06ld sec )\n",
        (double) send_bytes / elapse  / 1024.0 / 1024.0,
        send_bytes, diff.tv_sec, diff.tv_usec);

    return 0;
}

int child_proc(int connfd, int bufsize, int run_cpu)
{
    if (debug) {
        fprintfwt(stderr, "entering child_proc\n");
    }

    int n;
    unsigned char *buf;

    buf = malloc(bufsize);
    if (buf == NULL) {
        err(1, "malloc read buf in child_proc");
    }

    pid_t pid = getpid();
    fprintfwt(stderr, "server: pid: %d\n", pid);

    if (run_cpu != -1) {
        if (set_cpu(run_cpu) < 0) {
            errx(1, "set_cpu");
        }
    }

    for ( ; ; ) {
        n = read(connfd, buf, bufsize);
        if (debug) {
            fprintfwt(stderr, "read() returns\n");
        }

        if (n < 0) {
            if (errno == ECONNRESET) {
                fprintfwt(stderr, "server: connection reset by client\n");
                break;
            }
            else if (errno == EPIPE) {
                fprintfwt(stderr, "server: connection reset by client\n");
                break;
            }
            else {
                err(1, "read");
            }
        }
        if (n == 0) {
            fprintfwt(stderr, "child exit\n");
            return 0;
        }
    }

    return 0;
}

void sig_chld(int signo)
{
    pid_t pid;
    int   stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        ;
    }
    return;
}

int usage(void)
{
    char *msg =
"Usage: server [-b bufsize (4k)] [-p port (1234)] [-c run_cpu]\n"
"-b bufsize:    read buffer size (may add k for kilo, m for mega)\n"
"-p port:       port number (1234)\n"
"-c run_cpu:    specify server run cpu (in child proc)\n";

    fprintf(stderr, "%s", msg);

    return 0;
}

int main(int argc, char *argv[])
{
    int port = 1234;
    pid_t pid;
    struct sockaddr_in remote;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int listenfd;
    int c;
    int bufsize = 4096;
    int run_cpu = -1;

    while ( (c = getopt(argc, argv, "b:p:c:dh")) != -1) {
        switch (c) {
            case 'b':
                bufsize = get_num(optarg);
                break;
            case 'c':
                run_cpu = strtol(optarg, NULL, 0);
                break;
            case 'd':
                debug += 1;
                break;
            case 'h':
                usage();
                exit(0);
            case 'p':
                port = strtol(optarg, NULL, 0);
                break;
            default:
                break;
        }
    }
    argc -= optind;
    argv += optind;

    my_signal(SIGCHLD, sig_chld);
    my_signal(SIGPIPE, SIG_IGN);

    listenfd = tcp_listen(port);
    if (listenfd < 0) {
        errx(1, "tcp_listen");
    }

    for ( ; ; ) {
        int connfd = accept(listenfd, (struct sockaddr *)&remote, &addr_len);
        if (connfd < 0) {
            err(1, "accept");
        }
        
        pid = fork();
        if (pid == 0) { //child
            if (close(listenfd) < 0) {
                err(1, "close listenfd");
            }
            if (child_proc(connfd, bufsize, run_cpu) < 0) {
                errx(1, "child_proc");
            }
            exit(0);
        }
        else { // parent
            if (close(connfd) < 0) {
                err(1, "close for accept socket of parent");
            }
        }
    }
        
    return 0;
}
