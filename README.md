# Ayahesa

High performance scalable web framework. Ayahesa is loosely based on the MVC model and supports modern web features. The framework is build on top of [Kore](https://github.com/jorisvink/kore) application framework. The project is written in C99 and runs on most UNIX like operating systems.

Features:
* IPv6 support
* TLS support
* JWT authentication
* JSON-RPC pipeline
* PostgreSQL support
* Realtime websocket support
* Event source updates
* Basic templating

## Configuration

See `conf/framework.ini` and adjust for appropriate usage.

## Getting started

See `src/route.c` for example URIs pointing towards controllers defined in `src/controllers/`

## Building

In order to build the application [kore](https://github.com/jorisvink/kore) must be installed.
```
kore build
kore run
```
