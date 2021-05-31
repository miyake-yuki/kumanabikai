FLAGS1 = -fno-stack-protector -g -m32
FLAGS2 = -no-pie -z execstack
INGREDIENTS = hacking.c hacking-network.c

# tinywebは必ずsudoをつけて実行すること！
tinyweb: $(INGREDIENTS) tinyweb.c
	sudo sysctl -w kernel.randomize_va_space=0
	gcc $(FLAGS1) $(FLAGS2) $(INGREDIENTS) tinyweb.c -o tinyweb
	sudo chown root tinyweb
	sudo chmod u+s tinyweb

overflow: overflow_example.c
	sudo sysctl -w kernel.randomize_va_space=0
	gcc $(FLAGS1) overflow_example.c -o overflow

auth: auth_overflow.c
	sudo sysctl -w kernel.randomize_va_space=0
	gcc $(FLAGS1) auth_overflow.c -o auth

overwrite: overwrite_retaddr.c
	sudo sysctl -w kernel.randomize_va_space=0
	gcc $(FLAGS1) overwrite_retaddr.c -o overwrite

exploit: $(INGREDIENTS) tinyweb_exploit.c
	gcc -m32 $(INGREDIENTS) tinyweb_exploit.c -o exploit

attacker: tcp_client/main.go
	cd tcp_client && go build -o attacker
	mv -f tcp_client/attacker .

shell: shellcode.asm
	nasm shellcode.asm -o shell

bind: bindshell.asm
	nasm bindshell.asm -o bind

reverse: reverseshell.asm
	nasm reverseshell.asm -o reverse

.PHONY: clean
clean:
	sudo rm exploit tinyweb shell bind reverse attacker