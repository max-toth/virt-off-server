#pragma once

#include "core/ILogger.hpp"

#include <mutex>

/// Логирует диагностические сообщения в stdout/stderr.
///
/// Каждое сообщение предваряется уровнем в квадратных скобках:
///   [info], [warn], [error]
///
/// Вызовы синхронные, без буферизации — сообщение попадает в поток
/// немедленно. std::endl гарантирует flush после каждого вызова,
/// что важно при редиректе лога в файл или систему сбора логов
/// (Docker, journald).
///
/// info и warn пишут в stdout, error — в stderr, чтобы разделить
/// обычные сообщения и ошибки при анализе логов внешними системами.
///
/// Thread-safety: да — внутренний std::mutex защищает вывод.
class ConsoleLogger final : public ILogger {
public:
    void info(std::string_view msg) override;

    void warn(std::string_view msg) override;

    void error(std::string_view msg) override;

private:
    static std::mutex mtx_;
};
