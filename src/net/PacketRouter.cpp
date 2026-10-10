#include "net/PacketRouter.hpp"

/// Регистрирует handler для заданного первого байта пакета.
/// Если для этого типа уже был зарегистрирован handler — он заменяется.
void PacketRouter::register_handler(
    uint8_t type,
    std::function<void(udp::endpoint, std::span<const uint8_t>)> handler)
{
    handlers_[type] = std::move(handler);
}

/// Извлекает первый байт из data, ищет handler в таблице,
/// и если нашёлся — вызывает его с оставшейся частью данных (без первого байта).
/// Пустые пакеты молча игнорируются.
void PacketRouter::route(const udp::endpoint& from, std::span<const uint8_t> data)
{
    if (data.empty())
        return;

    auto it = handlers_.find(data[0]);
    if (it == handlers_.end())
        return;

    auto payload = data.subspan(1);
    it->second(from, payload);
}
