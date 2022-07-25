# Bundle static assets with nginx
FROM nginx:1.21.0-alpine as production
# Copy built assets from builder
WORKDIR /app

COPY . /usr/share/nginx/html
# Add your nginx.conf
COPY nginx.conf /etc/nginx/conf.d/default.conf
ENV PORT=80
# Expose port
EXPOSE 80
# Start nginx
CMD ["nginx", "-g", "daemon off;"]

