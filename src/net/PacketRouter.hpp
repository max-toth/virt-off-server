#pragma once

#include "net/IPacketRouter.hpp"

#include <unordered_map>

/// Диспатчеризирует пакеты по первому байту.
///
/// Внутри — обычный хеш-таблица: key = первый байт пакета, value = handler.
/// При route() первый байт отрезается (data.subspan(1)), и остаток
/// передаётся в зарегистрированный handler.
///
/// Если для первого байта нет handler — пакет молча дропается.
/// Это нормально для текущего протокола: мы знаем только 0xFF и 0xFE,
/// всё остальное (голосовые пакеты) тоже проходят через router как
/// зарегистрированный тип (голосовые пакеты не имеют специального
/// префикса — они relay'ятся через отдельный handler).
///
/// Thread-safety: не гарантируется — вызывается из одного io_context треда.
class PacketRouter final : public IPacketRouter {
public:
    void register_handler(uint8_t type,
                          std::function<void(udp::endpoint, std::span<const uint8_t>)> handler) override;

    void route(const udp::endpoint& from, std::span<const uint8_t> data) override;

private:
    std::unordered_map<uint8_t, std::function<void(udp::endpoint, std::span<const uint8_t>)>> handlers_;
};
