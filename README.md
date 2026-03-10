# Easy Windows HTTP Static File Server

The point of this is to have a quick and easy library for giving the primitives necessary to make an http server for static files in a given directory.

Based off of: https://learn.microsoft.com/en-us/windows/win32/http/http-server-sample-application

`http.c` and `mime.c` are the relevant files.

Look at `example.c` for usage. Run `example.exe` to serve the current directory, or `example.exe /some/path/here` for a different directory.
