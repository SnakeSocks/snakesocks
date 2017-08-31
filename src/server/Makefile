PROJECT=skserver

skserver: src/* 
	env GOPATH=${PWD} go build -o sksrv src/main/main.go
clean:
	rm sksrv bin pkg -rf
