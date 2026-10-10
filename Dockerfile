# ---- Build stage ----
FROM gcc:13 AS builder

WORKDIR /src
COPY . .

RUN apt-get update && apt-get install -y cmake && rm -rf /var/lib/apt/lists/*
RUN rm -rf build && cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --parallel

# ---- Run stage ----
FROM alpine:3.19

RUN apk add --no-cache libstdc++

COPY --from=builder /src/build/virt-off-server /usr/local/bin/virt-off-server

EXPOSE 7777/udp

ENTRYPOINT ["virt-off-server"]
CMD ["7777"]
