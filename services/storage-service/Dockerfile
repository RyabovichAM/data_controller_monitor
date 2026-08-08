FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake pkg-config \
        qt6-base-dev \
        librdkafka-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        libqt6core6 \
        librdkafka++1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build/storage-service /usr/local/bin/storage-service

VOLUME /data

ENTRYPOINT ["/usr/local/bin/storage-service"]
