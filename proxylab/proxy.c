#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

struct Uri
{
    char host[MAXLINE]; //hostname
    char port[MAXLINE]; //端口
    char path[MAXLINE]; //路径
};

void doit(int connfd);
void parse_uri(char *uri, struct Uri *uri_data);
void build_header(char *http_header, struct Uri *uri_data, rio_t *client_rio);
void *thread(void *vargp);

void sigpipe_handler(int sig)
{
    printf("sigpipe handled %d\n", sig);
}

int main(int argc, char **argv)
{
    int listenfd, *connfd;
    pthread_t tid;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

//    init_Cache();

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    Signal(SIGPIPE, sigpipe_handler);

    // 打开监听描述符
    listenfd = Open_listenfd(argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        // 为每个线程单独 malloc connfd
        // 由对应线程负责释放，防止并发问题
        connfd = Malloc(sizeof(int));
        // 接受请求
        *connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname,
                    MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        // 开一个线程来响应请求
        // 这是最简单的做法，更好的做法是写一个线程池，把任务提交给线程池
        Pthread_create(&tid, NULL, thread, connfd);
    }
}

void *thread(void *vargp) {
    int connfd = *((int *) vargp);
    // 据说用了这句就不需要自己手动清理线程了
    Pthread_detach(pthread_self());
    Free(vargp);
    doit(connfd);
    Close(connfd);
    return NULL;
}

void doit(int connfd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char server[MAXLINE];

    rio_t rio, server_rio;

    char cache_tag[MAXLINE];
    Rio_readinitb(&rio, connfd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);
    strcpy(cache_tag, uri);

    if (strcasecmp(method, "GET")) {
        printf("Proxy does not implement the method");
        return;
    }

    struct Uri *uri_data = (struct Uri *) malloc(sizeof(struct Uri));
    //判断uri是否缓存，若缓存，直接回复
//    int i;
//    if ((i = get_Cache(cache_tag)) != -1)
//    {
//        //加锁
//        P(&cache.data[i].mutex);
//        cache.data[i].read_cnt++;
//        if (cache.data[i].read_cnt == 1)
//            P(&cache.data[i].w);
//        V(&cache.data[i].mutex);
//
//        Rio_writen(connfd, cache.data[i].obj, strlen(cache.data[i].obj));
//
//        P(&cache.data[i].mutex);
//        cache.data[i].read_cnt--;
//        if (cache.data[i].read_cnt == 0)
//            V(&cache.data[i].w);
//        V(&cache.data[i].mutex);
//        return;
//    }

    // 解析uri
    parse_uri(uri, uri_data);

    // 设置header
    build_header(server, uri_data, &rio);

    //连接服务器
    int serverfd = Open_clientfd(uri_data->host, uri_data->port);
    if (serverfd < 0){
        printf("connection failed\n");
        return;
    }

    Rio_readinitb(&server_rio, serverfd);
    Rio_writen(serverfd, server, strlen(server));

//    char cache_buf[MAX_OBJECT_SIZE];
//    int size_buf = 0;
    size_t n;
    while ((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
        //注意判断是否会超出缓存大小
//        size_buf += n;
//        if(size_buf < MAX_OBJECT_SIZE)
//            strcat(cache_buf, buf);
        printf("proxy received %d bytes,then send\n", (int)n);
        Rio_writen(connfd, buf, n);
    }
    Close(serverfd);

//    if(size_buf < MAX_OBJECT_SIZE){
//        write_Cache(cache_tag, cache_buf);
//    }
}

void parse_uri(char *uri, struct Uri *uri_data) {
    char *hostpose = strstr(uri, "//");
    // 默认端口 80
    if (hostpose == NULL) {
        char *pathpose = strstr(uri, "/");
        if (pathpose != NULL)
            strcpy(uri_data->path, pathpose);
        strcpy(uri_data->port, "80");
        return;
    } else {
        char *portpose = strstr(hostpose + 2, ":");
        if (portpose != NULL) {
            int tmp;
            sscanf(portpose + 1, "%d%s", &tmp, uri_data->path);
            sprintf(uri_data->port, "%d", tmp);
            *portpose = '\0';
        } else {
            char *pathpose = strstr(hostpose + 2, "/");
            if (pathpose != NULL) {
                strcpy(uri_data->path, pathpose);
                strcpy(uri_data->port, "80");
                *pathpose = '\0';
            }
        }
        strcpy(uri_data->host, hostpose + 2);
    }
}

void build_header(char *http_header, struct Uri *uri_data, rio_t *client_rio) {
    const char *User_Agent = user_agent_hdr;
    char *conn_hdr = "Connection: close\r\n";
    char *prox_hdr = "Proxy-Connection: close\r\n";
    char *host_hdr_format = "Host: %s\r\n";
    char *requestlint_hdr_format = "GET %s HTTP/1.0\r\n";
    char *endof_hdr = "\r\n";

    char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXLINE], host_hdr[MAXLINE];
    sprintf(request_hdr, requestlint_hdr_format, uri_data->path);
    while (Rio_readlineb(client_rio, buf, MAXLINE) > 0) {
        if (strcmp(buf, endof_hdr) == 0)
            break; /*EOF*/

        if (!strncasecmp(buf, "Host", strlen("Host")))  {
            strcpy(host_hdr, buf);
            continue;
        }

        if (!strncasecmp(buf, "Connection", strlen("Connection")) && !strncasecmp(buf, "Proxy-Connection", strlen("Proxy-Connection")) && !strncasecmp(buf, "User-Agent", strlen("User-Agent"))) {
            strcat(other_hdr, buf);
        }
    }
    if (strlen(host_hdr) == 0) {
        sprintf(host_hdr, host_hdr_format, uri_data->host);
    }
    sprintf(http_header, "%s%s%s%s%s%s%s",
            request_hdr,
            host_hdr,
            conn_hdr,
            prox_hdr,
            User_Agent,
            other_hdr,
            endof_hdr);
}