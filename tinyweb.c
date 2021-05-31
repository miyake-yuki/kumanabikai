#include <stdio.h>
#include <fcntl.h>
#include <unistd.h> // 追加 read関数用
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "hacking.h"
#include "hacking-network.h"

#define PORT 80             // ユーザーが接続することになるポート
#define WEBPORT "./webroot" // ウェブサーバのルートディレクトリ

void handle_connection(int, struct sockaddr_in *); // ウェブリクエストを取り扱う
int get_file_size(int);                            // オープンするファイル記述子のファイルサイズを返す

int main(int argc, char const *argv[])
{
  int sockfd, new_sockfd;
  int yes = 1;
  struct sockaddr_in host_addr, client_addr; // 自らのアドレス情報
  socklen_t sin_size;

  printf("ポート:%d のウェブリクエストを受け付けます\n", PORT);

  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
  {
    fatal("ソケットが生成できませんでした");
  }
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
  {
    fatal("ソケットを SO_REUSEADDR に設定します");
  }

  host_addr.sin_family = AF_INET;         // ホストのバイト順
  host_addr.sin_port = htons(PORT);       // ネットワークバイト順の短整数
  host_addr.sin_addr.s_addr = INADDR_ANY; // 自動的に自分のIPを設定する
  memset(&(host_addr.sin_zero), '\0', 8); // 構造体の残りをゼロにセットする

  if (bind(sockfd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr)) == -1)
  {
    fatal("ソケットのバインドに失敗しました");
  }
  if (listen(sockfd, 20) == -1) // 20は保留中の接続のキューの最大長
  {
    fatal("ソケットの待受で失敗しました");
  }

  while (1) // 受付のループ
  {
    sin_size = sizeof(struct sockaddr);
    new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
    if (new_sockfd == -1)
    {
      fatal("コネクションの受付で失敗しました");
    }
    handle_connection(new_sockfd, &client_addr);
  }
  return 0;
}

/* この関数は,引き渡されたクライアントのアドレスからのソケットに対する接続を取り扱う
 * 接続はウェブリクエストとして処理され,この関数は接続されたソケット経由で応答を行う
 * 最終的に引き渡されたソケットは関数の最後でクローズされる
 */
void handle_connection(int sockfd, struct sockaddr_in *client_addr_ptr)
{
  unsigned char *ptr, request[500], resource[500];
  int fd, length;

  length = recv_line(sockfd, request);

  printf("%s:%d からリクエストを受け付けました \"%s\"\n", inet_ntoa(client_addr_ptr->sin_addr), ntohs(client_addr_ptr->sin_port), request);

  ptr = strstr(request, " HTTP/"); // 有効に見えるリクエストを検索する
  if (ptr == NULL)
  {
    printf(" HTTPではない!\n");
  }
  else
  {
    *ptr = 0;                             // URLの末尾でバッファを終端させる
    ptr = NULL;                           // ptrにNULLを設定(無効なリクエストのフラグとして使用)
    if (strncmp(request, "GET ", 4) == 0) // GETリクエスト
    {
      ptr = request + 4; // ptrはURLである
    }
    if (strncmp(request, "HEAD ", 5) == 0) // HEADリクエスト
    {
      ptr = request + 5; // ptrはURLである
    }

    if (ptr == NULL) // これは認識できるリクエストではない
    {
      printf("\t不明なリクエスト!\n");
    }
    else // 有効なリクエスト ptrがリソース名を指している
    {
      if (ptr[strlen(ptr) - 1] == '/')
      {
        strcat(ptr, "index.html"); // 末尾にindex.htmlを追加
      }
      strcpy(resource, WEBPORT); // リソースパスを追記
      strcat(resource, ptr);

      fd = open(resource, O_RDONLY, 0); // ファイルオープンを試みる
      printf("\t\'%s\' のオープン\t", resource);
      if (fd == -1) // ファイルが見つからない場合
      {
        printf(" 404 Not Found\n");
        send_string(sockfd, "HTTP/1.0 404 NOT FOUND\r\n");
        send_string(sockfd, "Server: Tiny web server\r\n\r\n");
        send_string(sockfd, "<html><head><title> 404 Not Found</title></head>");
        send_string(sockfd, "<body><h1>URL not found</h1></body></html>\r\n");
      }
      else // ファイルが存在している場合
      {
        printf(" 200 OK\n");
        send_string(sockfd, "HTTP/1.0 200 OK\r\n");
        send_string(sockfd, "Server: Tiny web server\r\n\r\n");
        if (ptr == request + 4) // GETリクエストの場合
        {
          if ((length = get_file_size(fd)) == -1)
          {
            fatal("リソースファイルの取得に失敗しました");
          }
          if ((ptr = (unsigned char *)malloc(length)) == NULL)
          {
            fatal("リソースファイル読み込み時のメモリ割り当てに失敗しました");
          }
          read(fd, ptr, length);        // ファイルをメモリ内に読み込む
          send(sockfd, ptr, length, 0); // ソケットにファイルの内容を送信
          free(ptr);                    // ファイルを格納したメモリ領域を開放
        }
        close(fd); // ファイルをクローズ
      }
    }
  }
  shutdown(sockfd, SHUT_RDWR); // ソケットをクローズ
}

/* この関数はオープンされているファイルの記述子を受け取り,対応しているファイルのサイズを返す
 * 失敗した際には-1を返す
 */
int get_file_size(int fd)
{
  struct stat stat_struct;
  if (fstat(fd, &stat_struct) == -1)
  {
    return -1;
  }
  return (int)stat_struct.st_size;
}
