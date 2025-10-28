# ConfigGUI Deployment Guide

**Version**: 2.0  
**Date**: October 28, 2025  
**Audience**: DevOps engineers, system administrators

---

## Table of Contents

1. [Build Systems](#build-systems)
2. [Desktop Deployment (Qt)](#desktop-deployment-qt)
3. [Web Server Deployment](#web-server-deployment)
4. [Docker Deployment](#docker-deployment)
5. [Production Configuration](#production-configuration)
6. [Monitoring and Maintenance](#monitoring-and-maintenance)
7. [Troubleshooting](#troubleshooting)

---

## Build Systems

### Prerequisites

#### Common
```bash
# Verify installations
cmake --version      # Should be 3.16+
git --version
g++ --version        # or clang++
```

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libyaml-dev \
    nlohmann-json3-dev
```

#### macOS
```bash
brew install cmake yaml

# For Qt GUI (optional)
brew install qt@6
```

#### Windows
- Download CMake from https://cmake.org/download/
- Install Visual Studio Build Tools 2019+
- Download Qt 6 from https://www.qt.io/download/

### CMake Configuration

**Step 1: Clone Repository**
```bash
git clone https://github.com/pvthanh/ConfigGUI.git
cd configGui
```

**Step 2: Create Build Directory**
```bash
mkdir build
cd build
```

**Step 3: Configure Build**

**Option A: Full Build (Qt + Server)**
```bash
cmake \
  -DBUILD_QT_APP=ON \
  -DBUILD_HTML_SERVER=ON \
  -DCMAKE_BUILD_TYPE=Release \
  ..
```

**Option B: Qt Only**
```bash
cmake \
  -DBUILD_QT_APP=ON \
  -DBUILD_HTML_SERVER=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  ..
```

**Option C: Server Only**
```bash
cmake \
  -DBUILD_QT_APP=OFF \
  -DBUILD_HTML_SERVER=ON \
  -DCMAKE_BUILD_TYPE=Release \
  ..
```

**Option D: Qt with Custom Qt Path**
```bash
cmake \
  -DCMAKE_PREFIX_PATH=/opt/qt6 \
  -DBUILD_QT_APP=ON \
  -DBUILD_HTML_SERVER=ON \
  -DCMAKE_BUILD_TYPE=Release \
  ..
```

### Compilation

**Linux/macOS**
```bash
# Build everything
cmake --build . --parallel 4

# Build specific target
cmake --build . --target ConfigGUI --parallel 4

# Build tests only
cmake --build . --target test_all
```

**Windows (MSVC)**
```bash
cmake --build . --config Release --parallel 4
```

### Testing

```bash
# Run all tests
ctest --output-on-failure

# Run specific test
ctest -R test_ini_parser --output-on-failure

# Run with verbose output
ctest -V
```

---

## Desktop Deployment (Qt)

### Building for Distribution

```bash
# 1. Configure for release
cmake -DCMAKE_BUILD_TYPE=Release \
       -DBUILD_QT_APP=ON \
       -DBUILD_HTML_SERVER=OFF \
       ..

# 2. Build
cmake --build . --parallel 4

# 3. Create package
cmake --install . --prefix ./install

# 4. Verify executable
./install/bin/ConfigGUI --version  # if implemented
```

### Creating Installers

#### Linux (AppImage)

```bash
# 1. Install appimagetool
wget https://github.com/AppImage/AppImageKit/releases/download/13/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage

# 2. Create AppImage
cmake --install . --prefix ./AppDir/usr
./appimagetool-x86_64.AppImage ./AppDir ConfigGUI.AppImage

# 3. Create launcher script
cat > ~/bin/configgui << 'EOF'
#!/bin/bash
~/ConfigGUI.AppImage
EOF
chmod +x ~/bin/configgui
```

#### macOS (DMG)

```bash
# 1. Create app bundle
cmake --install . --prefix ./install

# 2. Create DMG
hdiutil create -format UDBZ \
               -srcfolder ./install/bin/ConfigGUI.app \
               -volname "ConfigGUI" \
               ConfigGUI.dmg
```

#### Windows (Installer)

Use NSIS or MSI (requires additional tools):

```bash
# With NSIS
cpack -G NSIS

# Or with MSI
cpack -G WIX
```

### Distribution Checklist

- ✅ Test on target OS version
- ✅ Include required libraries (Qt runtime)
- ✅ Create desktop shortcut
- ✅ Add to application menu
- ✅ Include README and LICENSE
- ✅ Sign executable (if applicable)
- ✅ Create uninstaller

---

## Web Server Deployment

### Building for Production

```bash
# 1. Configure production build
cmake -DCMAKE_BUILD_TYPE=Release \
       -DBUILD_HTML_SERVER=ON \
       -DBUILD_QT_APP=OFF \
       ..

# 2. Build
cmake --build . --parallel 4

# 3. Verify executable
./src/html/ConfigGUIServer --help  # if implemented
```

### Running Server

**Development**
```bash
cd build
./src/html/ConfigGUIServer
# Server listens on http://localhost:8080
```

**Production (systemd)**

Create `/etc/systemd/system/configgui.service`:
```ini
[Unit]
Description=ConfigGUI Server
After=network.target

[Service]
Type=simple
User=configgui
WorkingDirectory=/opt/configgui
ExecStart=/opt/configgui/ConfigGUIServer
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl enable configgui
sudo systemctl start configgui
sudo systemctl status configgui
```

**Production (Supervisor)**

Create `/etc/supervisor/conf.d/configgui.conf`:
```ini
[program:configgui]
command=/opt/configgui/ConfigGUIServer
directory=/opt/configgui
user=configgui
autostart=true
autorestart=true
stderr_logfile=/var/log/configgui/error.log
stdout_logfile=/var/log/configgui/access.log
```

Enable and start:
```bash
sudo supervisorctl reread
sudo supervisorctl update
sudo supervisorctl start configgui
```

### Reverse Proxy (nginx)

```nginx
upstream configgui {
    server 127.0.0.1:8080;
}

server {
    listen 80;
    server_name config.example.com;
    
    client_max_body_size 100M;
    
    location / {
        proxy_pass http://configgui;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        # WebSocket support (if needed)
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
    }
}
```

Restart nginx:
```bash
sudo systemctl reload nginx
```

---

## Docker Deployment

### Building Docker Image

```bash
# Build image
docker build -f Dockerfile.html -t configgui:latest .

# Tag for registry
docker tag configgui:latest myregistry.azurecr.io/configgui:latest

# Push to registry
docker push myregistry.azurecr.io/configgui:latest
```

### Docker Compose

Create `docker-compose.yml`:
```yaml
version: '3.8'

services:
  configgui:
    image: configgui:latest
    ports:
      - "8080:8080"
    environment:
      LOG_LEVEL: info
    volumes:
      - ./configs:/app/configs
      - ./schemas:/app/schemas
    restart: unless-stopped

  nginx:
    image: nginx:alpine
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf:ro
      - ./ssl:/etc/nginx/ssl:ro
    depends_on:
      - configgui
    restart: unless-stopped
```

Run:
```bash
docker-compose up -d
```

### Running Container

**Development**
```bash
docker run -p 8080:8080 configgui:latest
```

**Production**
```bash
docker run \
  --name configgui \
  -d \
  -p 8080:8080 \
  -v /opt/configgui/configs:/app/configs \
  -v /opt/configgui/schemas:/app/schemas \
  --restart unless-stopped \
  configgui:latest
```

### Kubernetes Deployment

Create `configgui-deployment.yaml`:
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: configgui
spec:
  replicas: 3
  selector:
    matchLabels:
      app: configgui
  template:
    metadata:
      labels:
        app: configgui
    spec:
      containers:
      - name: configgui
        image: myregistry.azurecr.io/configgui:latest
        ports:
        - containerPort: 8080
        resources:
          requests:
            memory: "256Mi"
            cpu: "250m"
          limits:
            memory: "512Mi"
            cpu: "500m"
        volumeMounts:
        - name: configs
          mountPath: /app/configs
        - name: schemas
          mountPath: /app/schemas
      volumes:
      - name: configs
        emptyDir: {}
      - name: schemas
        emptyDir: {}

---
apiVersion: v1
kind: Service
metadata:
  name: configgui-service
spec:
  selector:
    app: configgui
  ports:
  - port: 8080
    targetPort: 8080
  type: LoadBalancer
```

Deploy:
```bash
kubectl apply -f configgui-deployment.yaml
kubectl get pods
kubectl get svc configgui-service
```

---

## Production Configuration

### Environment Variables

**Server Configuration**
```bash
export CONFIG_PORT=8080          # Server port
export CONFIG_HOST=0.0.0.0       # Listen address
export CONFIG_LOG_LEVEL=info     # Log level
export CONFIG_ENABLE_API=true    # Enable REST API
export CONFIG_ENABLE_UI=true     # Enable web UI
export CONFIG_MAX_UPLOAD=100M    # Max file upload size
```

### Security Configuration

**HTTPS/SSL**

```bash
# Generate self-signed certificate
openssl req -x509 -newkey rsa:4096 \
            -keyout key.pem \
            -out cert.pem \
            -days 365 -nodes

# Use with nginx reverse proxy for production
```

**Authentication (with nginx)**

```nginx
# Add HTTP Basic Auth
location / {
    auth_basic "ConfigGUI Admin";
    auth_basic_user_file /etc/nginx/.htpasswd;
    
    proxy_pass http://configgui;
}
```

Create `.htpasswd`:
```bash
htpasswd -c /etc/nginx/.htpasswd admin
```

### Performance Tuning

**nginx Configuration**
```nginx
# Worker processes
worker_processes auto;
worker_rlimit_nofile 65535;

# Connection settings
keepalive_timeout 65;
keepalive_requests 1000;

# Buffer sizes
client_body_buffer_size 10M;
client_max_body_size 100M;

# Gzip compression
gzip on;
gzip_min_length 1000;
gzip_types text/plain text/css application/json;
```

**Application Configuration**
```cpp
// In server initialization
server.set_connection_limit(1000);
server.set_read_timeout(30, 0);    // 30 seconds
server.set_write_timeout(30, 0);   // 30 seconds
server.set_keep_alive_max_count(100);
```

---

## Monitoring and Maintenance

### Health Checks

**HTTP Endpoint**
```bash
# Check if server is running
curl http://localhost:8080/api/health

# Response
{
  "status": "healthy",
  "version": "2.0",
  "uptime": 3600
}
```

**systemd Health**
```bash
sudo systemctl is-active configgui
```

### Logging

**Docker Logs**
```bash
docker logs configgui
docker logs -f configgui  # Follow logs
```

**systemd Logs**
```bash
journalctl -u configgui -f
journalctl -u configgui --since "2 hours ago"
```

**File Logging**
```bash
tail -f /var/log/configgui/server.log
```

### Backup and Recovery

**Backup Configurations**
```bash
# Backup server configs
tar -czf configgui_backup_$(date +%Y%m%d).tar.gz \
    /opt/configgui/configs/ \
    /opt/configgui/schemas/

# Store in S3
aws s3 cp configgui_backup_*.tar.gz s3://my-bucket/backups/
```

**Restore from Backup**
```bash
# Extract backup
tar -xzf configgui_backup_*.tar.gz -C /

# Verify
ls -la /opt/configgui/configs/
```

### Database Cleanup

```bash
# Remove old configuration files
find /opt/configgui/configs -mtime +30 -delete

# Archive configurations older than 90 days
tar -czf /archive/configs_old_$(date +%Y%m%d).tar.gz \
    $(find /opt/configgui/configs -mtime +90 -type f)
```

---

## Troubleshooting

### Port Already in Use

**Problem**: "Address already in use" error

**Solution**:
```bash
# Find process using port 8080
lsof -i :8080

# Kill process
kill -9 <PID>

# Or use different port
export CONFIG_PORT=8081
```

### Build Failures

**Problem**: "Qt6 not found"

**Solution**:
```bash
# Find Qt installation
find /opt -name "Qt6Config.cmake"

# Use specific path
cmake -DCMAKE_PREFIX_PATH=/opt/qt6 ..
```

**Problem**: "nlohmann/json.hpp not found"

**Solution**:
```bash
# Install via package manager
sudo apt-get install nlohmann-json3-dev

# Or manually
cd /usr/include
sudo wget https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
```

### Runtime Issues

**Problem**: Form doesn't load

**Solution**:
```bash
# Check browser console for errors
# Check server logs
journalctl -u configgui -f

# Verify API endpoint
curl http://localhost:8080/api/schemas
```

**Problem**: Upload fails

**Solution**:
```bash
# Check file permissions
ls -la /opt/configgui/configs/

# Fix permissions
sudo chown -R configgui:configgui /opt/configgui

# Check disk space
df -h /opt/configgui
```

### Performance Issues

**Problem**: Slow form loading

**Solution**:
```bash
# Check memory usage
top -p <server-pid>

# Restart service
sudo systemctl restart configgui

# Check large files
find /opt/configgui/configs -size +100M
```

---

## Upgrade Procedure

### Backup Current Version

```bash
# Create backup
cp -r /opt/configgui /opt/configgui.backup.$(date +%Y%m%d)

# Backup configurations
tar -czf /backup/configs_$(date +%Y%m%d).tar.gz \
    /opt/configgui/configs
```

### Build New Version

```bash
# Clone new version
git clone --branch v2.0 https://github.com/pvthanh/ConfigGUI.git
cd ConfigGUI
mkdir build && cd build

# Configure and build
cmake -DCMAKE_BUILD_TYPE=Release \
       -DBUILD_HTML_SERVER=ON \
       -DBUILD_QT_APP=OFF ..
cmake --build . --parallel 4
```

### Deploy New Version

```bash
# Stop service
sudo systemctl stop configgui

# Replace binary
sudo cp build/src/html/ConfigGUIServer /opt/configgui/

# Start service
sudo systemctl start configgui

# Verify
curl http://localhost:8080/api/health
```

### Rollback if Needed

```bash
# Stop service
sudo systemctl stop configgui

# Restore backup
rm -rf /opt/configgui
cp -r /opt/configgui.backup.20251028 /opt/configgui

# Start service
sudo systemctl start configgui
```

---

## Production Checklist

### Pre-Deployment
- ✅ All tests passing
- ✅ Performance testing completed
- ✅ Security review done
- ✅ Backup system configured
- ✅ Logging configured
- ✅ Monitoring alerts set up

### Deployment
- ✅ Backup current version
- ✅ Build in release mode
- ✅ Run unit tests
- ✅ Deploy to staging
- ✅ Smoke test in staging
- ✅ Deploy to production
- ✅ Verify all endpoints
- ✅ Check logs for errors

### Post-Deployment
- ✅ Monitor error logs
- ✅ Check performance metrics
- ✅ Verify backups working
- ✅ Update documentation
- ✅ Notify users
- ✅ Keep rollback ready for 2 hours

