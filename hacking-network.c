#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

// この関数はソケットファイル記述子、及び送信対象のnullで終端された
// 文字列へのポインタを受け取る。この関数は文字列の全バイトの送信を保証する.
// 成功時には1を返し、失敗時には0を返す
int send_string(int sockfd, char *buffer)
{
  int sent_bytes, bytes_to_send;

  bytes_to_send = strlen(buffer);
  while (bytes_to_send > 0)
  {
    sent_bytes = send(sockfd, buffer, bytes_to_send, 0);

    if (sent_bytes == -1)
    {
      return 0; // 失敗時には0を返す
    }

    bytes_to_send -= sent_bytes;
    buffer += sent_bytes;
  }
  return 1; // 成功時には1を返す
}

/* この関数はソケットファイル記述子と出力バッファへのポインタを受け取る
 * これはEOLバイト群に遭遇するまでソケットからデータを受信する
 * EOLバイトはソケットから読み取られるものの,出力バッファはEOLバイト群の直前で終端される
 * 読み込んだ行のサイズ(EOLバイトを除く)を返す
 */
int recv_line(int sockfd, char *dest_buffer)
{
#define EOL "\r\n"
#define EOL_SIZE 2

  unsigned char *ptr = dest_buffer;
  int eol_matched = 0;

  while (recv(sockfd, ptr, 1, 0) == 1) // 1バイト目を読み込む
  {
    if (*ptr == EOL[eol_matched]) // そのバイトは行末記号か？
    {
      eol_matched++;
      if (eol_matched == EOL_SIZE) // 行末記号全てに適合した場合、文字列終端を対応させる
      {
        *(ptr + 1 - EOL_SIZE) = '\0';
        return strlen(dest_buffer); // 受信したバイト群を返す
      }
    }
    else
    {
      eol_matched = 0;
    }
    ptr++; // 次のバイトを見つけるためにポインタをインクリスメントとする
  }
  return 0; // EOLが見つからなかった場合は0を返す
}