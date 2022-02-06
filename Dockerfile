FROM debian:stable-slim AS builder
RUN apt-get -y update
RUN apt-get install -y make cmake g++ libboost-program-options1.74-dev
COPY ./ /mnt
WORKDIR /mnt
ARG VER
RUN [ "make", "BUILD_TYPE=Release", "VERSION=${VER}", "clean", "all" ]

FROM debian:stable-slim AS final
COPY --from=builder /mnt/build/src/sfsdb-server /usr/local/bin/sfsdb-server
COPY --from=builder /mnt/build/src/sfsdb-cli /usr/local/bin/sfsdb-cli
VOLUME [ "/data" ]
EXPOSE 36363
ENTRYPOINT [ "sfsdb-server", "-v", "/data", "-p", "36363" ]
