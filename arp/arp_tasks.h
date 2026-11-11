#ifndef EMILI_ARP_TASKS_H
#define EMILI_ARP_TASKS_H

#include "arp_instance.h"

#include <cstddef>
#include <map>
#include <vector>

namespace arp {

// Representa una calle requerida sin elegir todavía su orientación.
// El ID es el mismo Link.id que ya asigna el lector.
struct ServiceTask {
    int id;

    // Identificadores originales de los nodos.
    int from;
    int to;

    // true: calle bi 
    //false: calle uni.
    bool reversible;

    // Tiempo original de recorrer la calle.
    double cost;

    // Línea de la instancia donde aparece la calle.
    int sourceLine;

    // Carga de la quinta columna; independiente del tiempo de recorrido.
    int demand;
};

// Representa la elección de una orientación para una tarea.
// Esta será la unidad de la futura secuencia del giant tour.
struct TaskVisit {
    int taskId;

    // false: from -> to.
    // true:  to -> from.
    bool reversed;
};

// Representa una tarea después de elegir su orientación.
struct OrientedTask {
    int taskId;

    // Nodos de entrada y salida según la orientación elegida.
    int from;
    int to;

    bool reversed;
    double cost;
    int demand;
};

class TaskCatalog {
public:
    // Construye el catálogo a partir de una instancia ya leída.
    explicit TaskCatalog(const ArpInstance& instance);

    // Devuelve todas las tareas requeridas.
    const std::vector<ServiceTask>& tasks() const;

    // Busca una tarea por su ID.
    const ServiceTask& byId(int taskId) const;

    // Interpreta una visita y comprueba que su orientación sea válida.
    OrientedTask resolve(const TaskVisit& visit) const;

    // Devuelve todas las orientaciones permitidas de una tarea.
    std::vector<OrientedTask> orientations(int taskId) const;

private:
    std::vector<ServiceTask> tasks_;

    // Relaciona el ID de tarea con su posición en tasks_.
    std::map<int, std::size_t> indexById_;
};

} // namespace arp

#endif