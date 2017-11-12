PROJECT=skserver

skserver: src/* 
	env GOPATH=${PWD} go build -tags netcgo -o sksrv src/main/main.go
clean:
	rm sksrv bin pkg -rf
