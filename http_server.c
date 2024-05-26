#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

void *client_proc(void *);

int main() {
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 10)) {
        perror("listen() failed");
        return 1;
    }

    while (1) {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);

        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, &client);
        pthread_detach(tid);
    }

    return 0;
}

void *client_proc(void *arg) {
    int client = *(int *)arg;
    char buf[2048];

    // Nhan du lieu tu client
    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        pthread_exit(NULL);
    }

    buf[ret] = 0;
    printf("Received from %d: %s\n", client, buf);

    char response[2048];
    double a, b, result = 0;
    char cmd[16];
    int matched;
    char operator[4];

    // xu li GET request
    if(strncmp(buf, "GET /?", 6) == 0)
    {
        char *query = strstr(buf, "/?") + 2;
        printf("Query string: %s\n", query);

        matched = sscanf(query, "a=%lf&b=%lf&cmd=%15s", &a, &b, cmd);
        printf("Matched: %d, a: %lf, b: %lf, cmd: %s\n", matched, a, b, cmd);
        if (matched == 3)
        {
            if (strcmp(cmd, "add") == 0)
            {
                result = a + b;
                strcpy(operator, "+");
            }
            else if (strcmp(cmd, "sub") == 0)
            {
                result = a - b;
                strcpy(operator, "-");
            }
            else if (strcmp(cmd, "mul") == 0)
            {
                result = a * b;
                strcpy(operator, "*");
            }
            else if (strcmp(cmd, "div") == 0)
            {
                if (b != 0)
                {
                    result = a / b;
                    strcpy(operator, "/");
                }
                else
                {
                    snprintf(response, sizeof(response),
                             "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                             "<html><body><h1>So Chia bang 0</h1></body></html>");
                    if (send(client, response, strlen(response), 0) < 0) {
                        perror("send() failed");
                    }
                    close(client);
                    pthread_exit(NULL);
                }
            }
            else
            {
                matched = 0; // cmd khong hop le
            }

            if (matched == 3)
            {
                snprintf(response, sizeof(response),
                         "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                         "<html><body>"
                         "<h1>Ket qua: %lf %s %lf = %lf</h1>"
                         "</body></html>",
                         a, operator, b, result);
            }
            else
            {
                snprintf(response, sizeof(response),
                         "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                         "<html><body><h1>Dau vao khong hop le</h1></body></html>");
            }
        }
        else
        {
            snprintf(response, sizeof(response),
                     "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                     "<html><body><h1>Dau vao khong hop le</h1></body></html>");
        }
    }
    // xu li POST request
    else if (strncmp(buf, "POST /", 6) == 0)
    {
        char *content = strstr(buf, "\r\n\r\n");
        if (content != NULL)
        {
            content += 4;
            printf("Content: %s\n", content);
            matched = sscanf(content, "a=%lf&b=%lf&cmd=%15s", &a, &b, cmd);
            printf("Matched: %d, a: %lf, b: %lf, cmd: %s\n", matched, a, b, cmd);
            if (matched == 3)
            {
                if (strcmp(cmd, "add") == 0)
                {
                    result = a + b;
                    strcpy(operator, "+");
                }
                else if (strcmp(cmd, "sub") == 0)
                {
                    result = a - b;
                    strcpy(operator, "-");
                }
                else if (strcmp(cmd, "mul") == 0)
                {
                    result = a * b;
                    strcpy(operator, "*");
                }
                else if (strcmp(cmd, "div") == 0)
                {
                    if (b != 0)
                    {
                        result = a / b;
                        strcpy(operator, "/");
                    }
                    else
                    {
                        snprintf(response, sizeof(response),
                                 "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                                 "<html><body><h1>So Chia bang 0</h1></body></html>");
                        send(client, response, strlen(response), 0);
                        close(client);
                        pthread_exit(NULL);
                    }
                }
                else
                {
                    matched = 0; // cmd khong hop le
                }

                if (matched == 3)
                {
                    snprintf(response, sizeof(response),
                             "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                             "<html><body>"
                             "<h1>Ket qua: %lf %s %lf = %lf</h1>"
                             "</body></html>",
                             a, operator, b, result);
                }
                else
                {
                    snprintf(response, sizeof(response),
                             "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                             "<html><body><h1>Dau vao khong hop le</h1></body></html>");
                }
            }
            else
            {
                snprintf(response, sizeof(response),
                         "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                         "<html><body><h1>Dau vao khong hop le</h1></body></html>");
            }
        }
        else
        {
            snprintf(response, sizeof(response),
                     "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                     "<html><body><h1>No content</h1></body></html>");
        }
    }
    else
    {
        snprintf(response, sizeof(response),
                 "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                 "<html><body><h1>Not Found</h1></body></html>");
    }

    send(client, response, strlen(response), 0);

    close(client);
    pthread_exit(NULL);
}
