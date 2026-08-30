FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake pkg-config \
        libpqxx-dev \
        librdkafka-dev \
        libprotobuf-dev libgrpc++-dev \
        protobuf-compiler protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
# The build context is the repository root: proto/ is shared by all services.
COPY proto/ proto/
COPY libs/ libs/
COPY services/storage-service/ services/storage-service/
RUN cmake -S services/storage-service -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        libpqxx-7.8 \
        librdkafka++1 \
        libprotobuf32t64 libgrpc++1.51t64 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build/storage-service /usr/local/bin/storage-service

VOLUME /data

EXPOSE 50052

ENTRYPOINT ["/usr/local/bin/storage-service"]
