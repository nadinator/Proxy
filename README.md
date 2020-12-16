# Proxy

### proxy.c
A concurrent proxy server that handles multiple client requests at a time. Implemented by creating a new thread for processing each client request, reaping each thread upon completion.

### webcache.c
A web cache that the proxy server uses to check for previous client requests. If any request is made, the proxy first checks the cache for the requested web content and returns it if found; otherwise, the proxy contacts the desired server, returns the content to the client, and caches it for possible future use. 
Uses an LRU eviction policy.
