FROM debian:bookworm-slim AS build
RUN apt-get update && apt-get install -y --no-install-recommends g++ cmake make && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON && cmake --build build --parallel && ctest --test-dir build --output-on-failure

FROM debian:bookworm-slim
RUN useradd --system --uid 10001 --create-home netask
WORKDIR /app
COPY --from=build /src/build/netask_server /app/netask_server
RUN mkdir -p /data && chown netask:netask /data
USER netask
ENV NETASK_DATA=/data/tasks.db
EXPOSE 5050/tcp
CMD ["/app/netask_server", "5050"]
