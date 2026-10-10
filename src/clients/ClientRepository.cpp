#include "clients/ClientRepository.hpp"

/// Регистрирует новый UDP-адрес в обоих индексах.
/// Возвращает уникальный числовой ID для этого клиента.
uint64_t ClientRepository::add(udp::endpoint ep) {
    auto id = next_id_++;
    by_id_[id] = ep;
    by_endpoint_[ep] = id;
    return id;
}

/// Удаляет клиента из обоих индексов.
/// Если ID не найден — ничего не делает (safe no-op).
void ClientRepository::remove(uint64_t id) {
    auto it = by_id_.find(id);
    if (it == by_id_.end()) return;

    by_endpoint_.erase(it->second);
    by_id_.erase(it);
}

/// Ищет ID клиента по его UDP-адресу.
/// Возвращает std::nullopt если адрес не зарегистрирован.
/// Используется в RelayHandler для определения отправителя.
std::optional<uint64_t> ClientRepository::find_by_endpoint(const udp::endpoint& ep) const {
    auto it = by_endpoint_.find(ep);
    if (it == by_endpoint_.end()) return std::nullopt;

    return it->second;
}

/// Обходит всех клиентов, вызывая fn(id, endpoint) для каждого,
/// ID которого НЕ равен exclude_id.
/// Используется в RelayHandler для широковещательной рассылки
/// голосовых пакетов всем, кроме отправителя.
void ClientRepository::for_each_except(uint64_t exclude_id,
                                       std::function<void(uint64_t, const udp::endpoint&)> fn) const {
    for (const auto& [id, ep] : by_id_) {
        if (id != exclude_id) fn(id, ep);
    }
}
