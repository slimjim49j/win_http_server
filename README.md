# Easy Windows HTTP Static File Server

The point of this is to have a quick and easy library for giving the primitives necessary to make an http server for static files in a given directory.

Based off of: https://learn.microsoft.com/en-us/windows/win32/http/http-server-sample-application

`http_server.c` and `mime.c` are the relevant files.

Look at `example.c` for usage. Run `example.exe` to serve the current directory, or `example.exe /some/path/here` for a different directory.

In order to serve on an interface other than localhost, the netsh http acl has to be updated, or the exe run with admin privs.
