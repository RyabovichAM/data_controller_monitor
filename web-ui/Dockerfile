FROM node:22-alpine AS build

WORKDIR /app

# Dependencies first: they change far less often than the sources, so this
# layer survives most rebuilds.
COPY web-ui/package.json web-ui/package-lock.json* ./
RUN npm install

COPY web-ui/ ./
RUN npm run build

FROM nginx:alpine

COPY web-ui/nginx.conf /etc/nginx/conf.d/default.conf
COPY --from=build /app/dist /usr/share/nginx/html

EXPOSE 80
