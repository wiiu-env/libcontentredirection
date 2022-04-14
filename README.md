[![Publish Docker Image](https://github.com/wiiu-env/libcontentredirection/actions/workflows/push_image.yml/badge.svg)](https://github.com/wiiu-env/libcontentredirection/actions/workflows/push_image.yml)

# libcontentredirection
Requires the [ContentRedirectionModule](https://github.com/wiiu-env/ContentRedirectionModule) to be running via [WUMSLoader](https://github.com/wiiu-env/WUMSLoader).  
Requires [wut](https://github.com/devkitPro/wut) for building.
Install via `make install`.

## Usage
Make sure to define 
```
WUMS_ROOT := $(DEVKITPRO)/wums
```
and add `-lcontentredirection` to `LIBS` and `$(WUMS_ROOT)` to `LIBDIRS`.

After that you can simply include `<content_redirection/redirection.h>`, call `ContentRedirection_Init();` to get access to the content redirection functions if it returns `CONTENT_REDIRECTION_RESULT_SUCCESS`.

## Use this lib in Dockerfiles.
A prebuilt version of this lib can found on dockerhub. To use it for your projects, add this to your Dockerfile.
```
[...]
COPY --from=wiiuenv/libcontentredirection:[tag] /artifacts $DEVKITPRO
[...]
```
Replace [tag] with a tag you want to use, a list of tags can be found [here](https://hub.docker.com/r/wiiuenv/libcontentredirection/tags). 
It's highly recommended to pin the version to the **latest date** instead of using `latest`.

## Format the code via docker

`docker run --rm -v ${PWD}:/src wiiuenv/clang-format:13.0.0-2 -r ./source ./include -i`
