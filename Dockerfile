# Multi-stage build for Weighted Weisfeiler-Lehman Engine
FROM alpine:3.19 AS builder

RUN apk add --no-cache \
    build-base \
    cmake \
    ninja

WORKDIR /app
COPY . .

RUN cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build \
    && cd build && ctest --output-on-failure

FROM alpine:3.19 AS runner

RUN apk add --no-cache libstdc++

WORKDIR /app
COPY --from=builder /app/build/wl_cli /usr/local/bin/wl_cli
COPY --from=builder /app/build/wl_benchmark /usr/local/bin/wl_benchmark
COPY --from=builder /app/build/wl_tests /usr/local/bin/wl_tests
COPY --from=builder /app/tests /app/tests
COPY --from=builder /app/data /app/data

ENTRYPOINT ["wl_cli"]
CMD ["-i", "tests/financial_network_pro.txt", "-v"]
