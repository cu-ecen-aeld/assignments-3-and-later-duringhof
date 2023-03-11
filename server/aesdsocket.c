#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <errno.h>

#define PORT "9000"
#define BACKLOG 10
#define CHUNK_SIZE 400
#define TEST_FILE "/var/tmp/aesdsocketdata"

int serv_sock_fd, client_sock_fd, output_file_fd, total_length, len, capacity, counter = 1, close_err;
struct sockaddr_in conn_addr;
char IP_addr[INET6_ADDRSTRLEN];

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void close_all()
{
    close(client_sock_fd);
    close(serv_sock_fd);
    close(output_file_fd);

    syslog(LOG_DEBUG, "Caught signal, exiting");
    closelog();

    remove(TEST_FILE);
}

// Signal handler for Signals SIGTERM and SIGINT
static void signal_handler(int signo)
{
    close_err = close(client_sock_fd);
    if (close_err == -1)
    {
        close_all();
        perror("close client socket error");
        exit(-1);
    }

    close_err = close(serv_sock_fd);
    if (close_err == -1)
    {
        close_all();
        perror("close server socket error");
        exit(-1);
    }

    close_err = close(output_file_fd);
    if (close_err == -1)
    {
        close_all();
        perror("close output file error");
        exit(-1);
    }

    syslog(LOG_DEBUG, "Closed connection from %s", IP_addr);
    syslog(LOG_DEBUG, "Caught signal, exiting");
    closelog();

    if (remove(TEST_FILE) == -1)
    {
        perror("file remove error");
    }
}
int main(int argc, char *argv[])
{
    openlog("aesdsocket", 0, LOG_USER);

    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        fprintf(stderr, "Cannot handle SIGINT\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGTERM, signal_handler) == SIG_ERR)
    {
        fprintf(stderr, "Cannot handle SIGTERM\n");
        exit(EXIT_FAILURE);
    }

    sigset_t socket_set;
    int rc = sigemptyset(&socket_set);
    if (rc != 0)
    {
        perror("signal empty set error");
        exit(-1);
    }
    rc = sigaddset(&socket_set, SIGINT);
    if (rc != 0)
    {
        perror("error adding signal SIGINT to set");
        exit(-1);
    }
    rc = sigaddset(&socket_set, SIGTERM);
    if (rc != 0)
    {
        perror("error adding signal SIGTERM to set");
        exit(-1);
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *res;

    if (getaddrinfo(NULL, PORT, &hints, &res) != 0)
    {
        perror("getaddrinfo error");
        exit(-1);
    }

    serv_sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serv_sock_fd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    int dummie = 1;
    if (setsockopt(serv_sock_fd, SOL_SOCKET, SO_REUSEADDR, &dummie, sizeof(int)) == -1)
    {
        perror("setsockopt error");
    }

    rc = bind(serv_sock_fd, res->ai_addr, res->ai_addrlen);
    if (rc == -1)
    {
        perror("bind error");
        exit(-1);
    }

    if (argc == 2 && (!strcmp(argv[1], "-d")))
    {
        pid_t customd;
        customd = fork();
        if (customd == -1)
        {
            perror("fork error");
            exit(-1);
        }
        else if (customd != 0)
        {
            exit(EXIT_SUCCESS);
        }

        if (setsid() == -1)
        {
            perror("setsid");
            exit(-1);
        }

        if (chdir("/") == -1)
        {
            exit(-1);
        }

        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);
    }

    output_file_fd = open(TEST_FILE, O_CREAT | O_RDWR, 0644);
    if (output_file_fd == -1)
    {
        perror("error opening file at /var/temp/aesdsocketdata");
        exit(-1);
    }

    if (listen(serv_sock_fd, BACKLOG) == -1)
    {
        perror("error listening");
        exit(-1);
    }
    freeaddrinfo(res);
    while (1)
    {

        socklen_t conn_addr_len = sizeof(conn_addr);
        client_sock_fd = accept(serv_sock_fd, (struct sockaddr *)&conn_addr, &conn_addr_len);
        if (client_sock_fd == -1)
        {
            close_all();
            exit(-1);
        }

        inet_ntop(AF_INET, get_in_addr((struct sockaddr *)&conn_addr), IP_addr, sizeof(IP_addr));
        syslog(LOG_DEBUG, "Accepted connection from %s", IP_addr);

        char *buf_data = calloc(CHUNK_SIZE, sizeof(char));
        if (buf_data == NULL)
        {
            syslog(LOG_ERR, "Error: malloc failed");
            close_all();
            exit(-1);
        }
        int loc = 0;

        int merr = sigprocmask(SIG_BLOCK, &socket_set, NULL);
        if (merr == -1)
        {
            perror("sigprocmask block error");
            close_all();
            exit(-1);
        }

        while ((len = recv(client_sock_fd, buf_data + loc, CHUNK_SIZE, 0)) > 0)
        {
            if (len == -1)
            {
                perror("recv error");
                close_all();
                exit(-1);
            }

            if (strchr(buf_data, '\n') != NULL)
                break;
            loc += len;
            counter++;
            buf_data = (char *)realloc(buf_data, ((counter * CHUNK_SIZE) * sizeof(char)));
            if (buf_data == NULL)
            {
                syslog(LOG_ERR, "Error: realloc failed");
                close_all();
                exit(-1);
            }
        }

        int werr = write(output_file_fd, buf_data, strlen(buf_data));
        if (werr == -1)
        {
            perror("write error");
            close_all();
            exit(-1);
        }

        total_length += (strlen(buf_data));
        lseek(output_file_fd, 0, SEEK_SET);
        char *send_data_buf = calloc(total_length, sizeof(char));

        int rerr = read(output_file_fd, send_data_buf, total_length);
        syslog(LOG_DEBUG, "%d", total_length);
        if (rerr == -1)
        {
            perror("read error");
            close_all();
            exit(-1);
        }

        syslog(LOG_DEBUG, "%s", send_data_buf);
        int serr = send(client_sock_fd, send_data_buf, total_length, 0);
        if (serr == -1)
        {
            close_all();
            perror("send error");
            exit(-1);
        }

        free(buf_data);
        free(send_data_buf);

        merr = sigprocmask(SIG_UNBLOCK, &socket_set, NULL);
        if (merr == -1)
        {
            perror("sigprocmask unblock error");
            close_all();
            exit(-1);
        }
    }
    return 0;
}
