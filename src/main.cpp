#include "core/app.h"
#include "utils/logger.h"
#include <iostream>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#endif

App* g_app = nullptr;

void signalHandler(int signal) {
    if (g_app) {
        Logger::info("收到信号：" + std::to_string(signal) + "，正在关闭应用程序");
        g_app->stop();
    }

    Logger::shutdown();
    exit(signal);
}

int main(int argc, char** argv) {
    try {
        Logger::initialize("yolo_fps_assist.log");
        Logger::info("应用程序启动");

        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        g_app = new App();

        if (!g_app->initialize()) {
            Logger::error("应用程序初始化失败");
            delete g_app;
            Logger::shutdown();
            return 1;
        }

        g_app->run();

        std::cout << "按 F12 退出程序" << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

#ifdef _WIN32
            if (GetAsyncKeyState(VK_F12) & 0x8000) {
                break;
            }
#endif
        }

        g_app->stop();
        delete g_app;
        g_app = nullptr;

        Logger::info("应用程序正常退出");
        Logger::shutdown();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "未处理的异常：" << e.what() << std::endl;

        if (g_app) {
            delete g_app;
            g_app = nullptr;
        }

        Logger::critical("未处理的异常：" + std::string(e.what()));
        Logger::shutdown();

        return 1;
    }
}