package main

/*
#cgo LDFLAGS: -Lgtgo/linux -lgtgo
#include <stdlib.h>
#include "gtgo/include/gtgo.h"
*/
import "C"

import (
	"fmt"
	"io"
	"net/http"
	"unsafe"
)

const decryptionKey = "0123456789abcdef"

func decrypt(input string) (string, error) {
	cInput := C.CString(input)
	defer C.free(unsafe.Pointer(cInput))

	cKey := C.CString(decryptionKey)
	defer C.free(unsafe.Pointer(cKey))

	var cOutput *C.char

	// Call the C function
	C.fatal_dec(cInput, &cOutput, cKey)
	defer C.free(unsafe.Pointer(cOutput))

	// if ret != 0 {
	// 	return "", fmt.Errorf("decryption failed with code %d", int(ret))
	// }

	return C.GoString(cOutput), nil
}

func handler(w http.ResponseWriter, r *http.Request) {
	defer r.Body.Close()
	body, err := io.ReadAll(r.Body)
	if err != nil {
		http.Error(w, "failed to read body", http.StatusBadRequest)
		return
	}

	fmt.Println("Received encrypted data:", string(body))

	decrypted, err := decrypt(string(body))
	if err != nil {
		http.Error(w, "decryption failed: "+err.Error(), http.StatusInternalServerError)
		return
	}
	fmt.Println("Decrypted data:", decrypted)

	w.Write([]byte("KILL_PROGRAM"))
}

func main() {
	http.HandleFunc("/decrypt", handler)
	fmt.Println("Server listening on :8000")
	http.ListenAndServe(":8000", nil)
}
