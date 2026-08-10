FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake pkg-config \
        qt6-base-dev qt6-serialport-dev \
        librdkafka-dev \
        libprotobuf-dev libgrpc++-dev \
        protobuf-compiler protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
# The build context is the repository root: proto/ is shared by all services.
COPY proto/ proto/
COPY services/collector/ services/collector/
RUN cmake -S services/collector -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        libqt6core6 libqt6network6 libqt6serialport6 \
        librdkafka++1 \
        libprotobuf32t64 libgrpc++1.51t64 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build/collector /usr/local/bin/collector

ENTRYPOINT ["/usr/local/bin/collector"]
