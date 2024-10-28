package main

import (
	"fmt"
	"os"
	"os/signal"
	"syscall"
	"time"
)

func main() {
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)

	go func() {
		for {
			sig := <-sigs
			fmt.Printf("Получен сигнал: %s\n", sig)
		}
	}()

	fmt.Println("Приложение запущено. Нажмите Ctrl+C для завершения.")
	for {
		time.Sleep(time.Second)
	}
}
