package server

import (
	"net"
	"strconv"
)

var (
	MaxWorker = "1024"  //os.Getenv("MAX_WORKERS")
	MaxQueue  = "10240" //os.Getenv("MAX_QUEUE")
)

type Job struct {
	Payload net.Conn
}

var JobQueue chan Job

type Worker struct {
	WorkerPool chan chan Job
	JobChannel chan Job
	quit       chan struct{}
}

func NewWorker(workerPool chan chan Job) Worker {
	return Worker{
		WorkerPool: workerPool,
		JobChannel: make(chan Job),
		quit:       make(chan struct{})}
}

func (w Worker) Start(s *Server) {
	go func() {
		for {
			w.WorkerPool <- w.JobChannel

			select {
			case job := <-w.JobChannel:
				s.handle(job.Payload)
			case <-w.quit:
				return
			}
		}
	}()
}

func (w Worker) Stop() {
	go func() {
		w.quit <- zero
	}()
}

type Dispatcher struct {
	WorkerPool chan chan Job
	MaxWorker  int
}

func NewDispatcher() *Dispatcher {
	maxw, _ := strconv.Atoi(MaxWorker)
	pool := make(chan chan Job, maxw)
	return &Dispatcher{WorkerPool: pool, MaxWorker: maxw}
}

func (d *Dispatcher) Run(s *Server) {
	for i := 0; i < d.MaxWorker; i++ {
		worker := NewWorker(d.WorkerPool)
		worker.Start(s)
	}
	go d.dispatch()
}

func (d *Dispatcher) dispatch() {
	for {
		select {
		case job := <-JobQueue:
			go func(job Job) {
				jobChannel := <-d.WorkerPool
				jobChannel <- job
			}(job)
		}
	}
}
