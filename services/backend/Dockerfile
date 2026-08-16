FROM ubuntu:24.04 AS build

# The packaged Drogon was built with every database backend it supports, so its
# CMake config insists on finding all of them even though nothing here uses the
# ORM — hence the pq/sqlite/mysql/hiredis/yaml development packages.
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake pkg-config \
        libdrogon-dev libjsoncpp-dev uuid-dev zlib1g-dev libbrotli-dev libboost-dev \
        libpq-dev libsqlite3-dev libmysqlclient-dev libhiredis-dev libyaml-cpp-dev \
        librdkafka-dev \
        libprotobuf-dev libgrpc++-dev \
        protobuf-compiler protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
# The build context is the repository root: proto/ and libs/ are shared.
COPY proto/ proto/
COPY libs/ libs/
COPY services/backend/ services/backend/

# Drogon's FindMySQL looks for library names Ubuntu stopped shipping
# (mysqlclient_r, mariadbclient), so it is pointed at the one that is there.
RUN cmake -S services/backend -B build -DCMAKE_BUILD_TYPE=Release \
        -DMYSQL_INCLUDE_DIRS=/usr/include/mysql \
        -DMYSQL_LIBRARIES=/usr/lib/x86_64-linux-gnu/libmysqlclient.so \
    && cmake --build build -j$(nproc)

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        libdrogon1t64 libtrantor1 libjsoncpp25 \
        librdkafka++1 \
        libprotobuf32t64 libgrpc++1.51t64 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build/backend /usr/local/bin/backend

EXPOSE 8080

ENTRYPOINT ["/usr/local/bin/backend"]
