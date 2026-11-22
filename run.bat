@echo off
REM ============================================================
REM run.bat - One-click launcher for Campus Navigation System
REM Run from D:\PathNavigationSystem\
REM ============================================================

echo [INFO] Starting Campus Navigation System...
echo [INFO] Backend API bridge starting on http://localhost:5000
echo [INFO] Press Ctrl+C to stop the server.
echo.

REM Install backend deps if needed
if not exist "backend\node_modules" (
    echo [INFO] Installing backend dependencies...
    cd backend
    npm install
    cd ..
)

REM Start the backend server (serves frontend + API)
start "Campus Nav Backend" /D "%~dp0" node backend/server.js

REM Wait 2 seconds then open browser
timeout /t 2 /nobreak >nul
start "" "http://localhost:5000"

echo [INFO] Server is running. Browser opened at http://localhost:5000
echo [INFO] Close the "Campus Nav Backend" window to stop.
