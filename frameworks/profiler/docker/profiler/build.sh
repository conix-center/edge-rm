# if error, run "docker buildx create --use"
docker buildx build --platform linux/amd64,linux/arm64,linux/arm/v6 -t "jnoor/profiler:v1" --push .
