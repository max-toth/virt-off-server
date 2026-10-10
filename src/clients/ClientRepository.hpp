#pragma once

#include "clients/IClientRepository.hpp"

#include <unordered_map>

/// Хранит подключённых клиентов с двунаправленным индексом.
///
/// Внутри два словаря:
///   by_id_       — быстрый поиск endpoint по ID для отправки конкретному клиенту
///   by_endpoint_ — быстрый поиск ID по endpoint для определения отправителя
///
/// ID генерируется внутри через next_id_ (start=1) и не зависит от
/// MonotonicIdGenerator — репозиторий сам решает, какой ID назначить.
/// Это сделано намеренно: если в будущем появится другой источник ID
/// (например, UUID или ID от внешней аутентификации), репозиторий
/// останется неизменным, а логика ID будет в отдельном компоненте.
///
/// Важно: при remove() удаляется запись из обоих словарей, чтобы
/// не было "висячих" ссылок.
///
/// Thread-safety: не гарантируется — вызывается из одного io_context треда.
/// Future: при переходе на multi-thread нужно добавить мьютекс или
/// заменить unordered_map на concurrent_map.
class ClientRepository final : public IClientRepository {
public:
    uint64_t add(udp::endpoint ep) override;

    void remove(uint64_t id) override;

    std::optional<uint64_t> find_by_endpoint(const udp::endpoint& ep) const override;

    void for_each_except(uint64_t exclude_id, std::function<void(uint64_t, const udp::endpoint&)> fn) const override;

private:
    std::unordered_map<uint64_t, udp::endpoint> by_id_;

    std::unordered_map<udp::endpoint, uint64_t> by_endpoint_;

    /// Счётчик для назначения ID. Стартует с 1 (0 зарезервирован за сервером).
    uint64_t next_id_ = 1;
};
