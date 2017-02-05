# Ayahesa
[![Build Status](https://travis-ci.org/yorickdewid/ayahesa.svg?branch=master)](https://travis-ci.org/yorickdewid/ayahesa)

High performance scalable web framework. Ayahesa is loosely based on the MVC model and supports modern web features. The framework is build on top of [Kore](https://github.com/jorisvink/kore) application framework. The project is written in C99 and runs on most UNIX like operating systems.

Features:
* MVC based
* JWT authentication
* JSON-RPC pipeline
* PostgreSQL database connector
* Realtime websockets
* Templating
* File management
* Opque encryption
* IPv6 support
* TLS support

## Configuration

See `conf/framework.ini` and adjust for appropriate usage.

## Getting started

`src/route.c` defines routes and points towards controllers defined in `src/controllers/`

`src/middleware.c` serves as middleware between routes and controllers. This is the place to declare authentication validators.

`src/trigger.c` can act on framework fired events such as authentication failure.

`src/provider.c` initalize additional components.

## Building

In order to build the application [kore](https://github.com/jorisvink/kore) must be installed.
```
kore build
kore run
```
