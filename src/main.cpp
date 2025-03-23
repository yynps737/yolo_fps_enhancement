#include "core/app.h"
#include "utils/logger.h"
#include <iostream>
#include <signal.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef _WIN32
#include <windows.h>
#endif

App* g_app = nullptr;
std::atomic<bool> g_running(true);
std::mutex g_mutex;
std::condition_variable g_cv;

void signalHandler(int signal) {
    if (g_app) {
        Logger::info("收到信号：" + std::to_string(signal) + "，正在安全关闭应用程序");
        try {
            g_app->stop();
        } catch (const std::exception& e) {
            Logger::error("停止应用时出错: " + std::string(e.what()));
        }
    }

    g_running.store(false);
    g_cv.notify_all();

    try {
        Logger::shutdown();
    } catch (...) {
    }

    std::exit(signal);
}

#ifndef _WIN32
void exitCheckThread() {
    std::cout << "按 q 然后回车键退出程序" << std::endl;

    char c;
    while (std::cin.get(c)) {
        if (c == 'q' || c == 'Q') {
            g_running.store(false);
            g_cv.notify_all();
            break;
        }
    }
}
#endif

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

#ifdef _WIN32
        std::cout << "按 F12 退出程序" << std::endl;

        while (g_running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (GetAsyncKeyState(VK_F12) & 0x8000) {
                g_running.store(false);
                break;
            }
        }
#else
        std::thread exitThread(exitCheckThread);

        {
            std::unique_lock<std::mutex> lock(g_mutex);
            g_cv.wait(lock, []{ return !g_running.load(); });
        }

        if (exitThread.joinable()) {
            exitThread.join();
        }
#endif

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