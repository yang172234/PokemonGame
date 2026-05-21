@echo off
title 宝可梦对战系统启动器
color 0A

:MENU
cls
echo ========================================
echo      宝可梦对战系统 - 启动菜单
echo ========================================
echo.
echo   [1] 启动游戏（推荐）
echo   [2] 仅启动后端服务
echo   [3] 仅启动前端界面
echo   [4] 安装/更新依赖
echo   [5] 下载游戏图片
echo   [6] 退出
echo.
set /p choice="请选择操作 (1-6): "

if "%choice%"=="1" goto START_GAME
if "%choice%"=="2" goto START_BACKEND
if "%choice%"=="3" goto START_FRONTEND
if "%choice%"=="4" goto INSTALL_DEPS
if "%choice%"=="5" goto DOWNLOAD_IMAGES
if "%choice%"=="6" goto EXIT
goto MENU

:START_GAME
cls
echo [启动] 正在启动游戏...
echo.
echo 正在启动后端服务器...
start "宝可梦后端" cmd /k "node server.js"
timeout /t 2 /nobreak >nul
echo 正在启动前端界面...
start "宝可梦前端" cmd /k "npx vite"
timeout /t 3 /nobreak >nul
echo 正在打开浏览器...
start http://localhost:5173
echo.
echo 游戏启动成功！关闭命令行窗口可停止服务。
pause
goto MENU

:START_BACKEND
cls
echo 启动后端服务器...
start "宝可梦后端" cmd /k "node server.js"
echo 后端服务已启动在 http://localhost:3000
pause
goto MENU

:START_FRONTEND
cls
echo 启动前端界面...
start "宝可梦前端" cmd /k "npx vite"
echo 前端界面启动后请访问 http://localhost:5173
pause
goto MENU

:INSTALL_DEPS
cls
echo 正在安装依赖...
call npm install
call npm install vue@latest vite @vitejs/plugin-vue axios --save-dev
echo 依赖安装完成！
pause
goto MENU

:DOWNLOAD_IMAGES
cls
echo 正在下载精灵图片...
if exist "download-images-fallback.js" (
    node download-images-fallback.js
) else (
    echo 未找到下载脚本，请手动下载图片
)
pause
goto MENU

:EXIT
exit