package main
 
import (
    "fmt"
    "log"
    "io/ioutil"
    "net/http"
)
 
func main() {
 
    // API routes
    http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
        fmt.Fprintf(w, "Hello world from GfG")
    })
    http.HandleFunc("/hi", func(w http.ResponseWriter, r *http.Request) {
        fmt.Fprintf(w, "Hi")
    })
    http.HandleFunc("/catalog", func(w http.ResponseWriter, r *http.Request) {
        fileBytes, err := ioutil.ReadFile("catalog-dev.json")
    if err != nil {
        panic(err)
    }
    w.WriteHeader(http.StatusOK)
    w.Header().Set("Content-Type", "application/json")
    w.Write(fileBytes)
    })
    http.HandleFunc("/checkin", func(w http.ResponseWriter, r *http.Request) {
        fileBytes, err := ioutil.ReadFile("checkin1")
        if err != nil {
                panic(err)
        }
        w.WriteHeader(http.StatusOK)
        w.Header().Set("Content-Type", "application/json")
        w.Write(fileBytes)
    }) 
    http.HandleFunc("/cisco-secure-client-macos-cloudmanagement-testpackage1-1.0.pkg", func(w http.ResponseWriter, r *http.Request) {
        fileBytes, err := ioutil.ReadFile("cisco-secure-client-macos-cloudmanagement-testpackage1-1.0.pkg")
        if err != nil {
                panic(err)
        }
        w.WriteHeader(http.StatusOK)
        w.Header().Set("Content-Type", "application/octet-stream")
        w.Write(fileBytes)
    }) 
    port := ":5000"
    fmt.Println("Server is running on port" + port)
 
    // Start server on port specified above
    log.Fatal(http.ListenAndServe(port, nil))
}
