# ESP8266 Open RTOS HTTP upload

This module adds support for uploading files using HTTP.

###Usage

```
cd esp-open-rtos/extras
git clone https://github.com/kanflo/eor-http-upload.git http-upload
```

Include the driver in your project makefile as any other extra component:

```
EXTRA_COMPONENTS=extras/http-upload
```

See ```example/http_upload.c``` for a complete example while noting that the makefile depends on the environment variable ```$EOR_ROOT``` pointing to your ESP8266 Open RTOS root.
