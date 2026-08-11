FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake pkg-config \
        libpqxx-dev \
        libprotobuf-dev libgrpc++-dev \
        protobuf-compiler protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
# The build context is the repository root: proto/ is shared by all services.
COPY proto/ proto/
COPY services/config-service/ services/config-service/
RUN cmake -S services/config-service -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        libpqxx-7.8 \
        libprotobuf32t64 libgrpc++1.51t64 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build/config-service /usr/local/bin/config-service

EXPOSE 50051

ENTRYPOINT ["/usr/local/bin/config-service"]
