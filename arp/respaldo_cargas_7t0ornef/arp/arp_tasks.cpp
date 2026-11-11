#include "arp_tasks.h"
#include <cmath>
#include <stdexcept>
#include <string>

namespace arp {

TaskCatalog::TaskCatalog(const ArpInstance& instance) {
    // Recorre todas las calles leídas.
    for (const auto& link : instance.links) {

        // Las calles opcionales permanecen en el grafo,
        // pero no se convierten en servicios obligatorios.
        if (!link.required) {
            continue;
        }

        // El ID debe ser positivo y no puede repetirse.
        if (link.id <= 0 || indexById_.count(link.id)) {
            throw std::runtime_error(
                "ID de tarea invalido o repetido: "
                + std::to_string(link.id)
            );
        }

        // El costo debe ser finito y no negativo.
        if (!std::isfinite(link.cost) || link.cost < 0) {
            throw std::runtime_error(
                "Costo invalido en tarea "
                + std::to_string(link.id)
            );
        }

        // Guarda la posición de la tarea dentro del vector.
        indexById_[link.id] = tasks_.size();

        // Conserva el ID original de la calle.
        // Una calle bi permite inversión; una uni no.
        tasks_.push_back({
            link.id,
            link.from,
            link.to,
            !link.directed,
            link.cost,
            link.sourceLine
        });
    }
}

const std::vector<ServiceTask>& TaskCatalog::tasks() const {
    return tasks_;
}

const ServiceTask& TaskCatalog::byId(int taskId) const {
    const auto found = indexById_.find(taskId);

    if (found == indexById_.end()) {
        throw std::runtime_error(
            "No existe la tarea requerida "
            + std::to_string(taskId)
        );
    }

    return tasks_.at(found->second);
}

OrientedTask TaskCatalog::resolve(const TaskVisit& visit) const {
    const ServiceTask& task = byId(visit.taskId);

    // Una calle uni no permite invertir el sentido de servicio.
    if (visit.reversed && !task.reversible) {
        throw std::runtime_error(
            "La tarea uni "
            + std::to_string(task.id)
            + " no permite orientacion inversa"
        );
    }

    // Si reversed es true, intercambia entrada y salida.
    return {
        task.id,
        visit.reversed ? task.to : task.from,
        visit.reversed ? task.from : task.to,
        visit.reversed,
        task.cost
    };
}

std::vector<OrientedTask> TaskCatalog::orientations(int taskId) const {
    const ServiceTask& task = byId(taskId);

    std::vector<OrientedTask> result;

    // Toda tarea permite el sentido original.
    result.push_back(resolve({taskId, false}));

    // Solo una tarea bi permite el sentido inverso.
    if (task.reversible) {
        result.push_back(resolve({taskId, true}));
    }

    return result;
}

} // namespace arp