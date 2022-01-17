FROM ubuntu:latest

COPY build/netest /

ENTRYPOINT /netest

