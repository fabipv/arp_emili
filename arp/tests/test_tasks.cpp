#include "arp_tasks.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void check(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename Action>
void rejects(Action action, const std::string& message) {
    bool rejected = false;

    try {
        action();
    } catch (const std::runtime_error&) {
        rejected = true;
    }

    check(rejected, message);
}

} // namespace

int main() {
    try {
        arp::ArpInstance instance;

        // Campos de Link:
        // id, from, to, directed, cost,
        // required, demand, sourceLine.
        //
        // Se usan IDs no consecutivos para comprobar
        // que no se confundan con posiciones del vector.
        instance.links = {
            {10, 100, 200, false, 7.5, true, 1, 10},
            {20, 200, 300, true, 2.0, true, 1, 11},
            {30, 300, 100, false, 1.0, false, 0, 12},
            {40, 100, 200, false, 9.0, true, 1, 13}
        };

        const arp::TaskCatalog catalog(instance);

        check(
            catalog.tasks().size() == 3,
            "Solo las requeridas son tareas"
        );

        check(
            catalog.tasks()[2].id == 40,
            "Se conserva el ID original"
        );

        const auto options = catalog.orientations(10);

        check(
            options.size() == 2,
            "bi tiene dos orientaciones"
        );

        check(
            options[0].taskId == options[1].taskId,
            "Ambas orientaciones pertenecen a la misma tarea"
        );

        check(
            options[0].from == 100
                && options[0].to == 200
                && !options[0].reversed,
            "Orientacion directa"
        );

        check(
            options[1].from == 200
                && options[1].to == 100
                && options[1].reversed
                && options[1].cost == 7.5,
            "Orientacion inversa conserva el costo"
        );

        check(
            catalog.orientations(20).size() == 1,
            "uni tiene una orientacion"
        );

        rejects(
            [&] { catalog.resolve({20, true}); },
            "No invertir uni"
        );

        rejects(
            [&] { catalog.byId(30); },
            "Opcional no es tarea"
        );

        rejects(
            [&] { catalog.byId(999); },
            "Rechazar ID inexistente"
        );

        check(
            catalog.byId(40).cost == 9.0,
            "Conservar calles paralelas como tareas distintas"
        );

        instance.links.push_back(instance.links.front());

        rejects(
            [&] {
                arp::TaskCatalog duplicate(instance);
            },
            "Rechazar ID duplicado"
        );

        arp::ArpInstance empty;

        check(
            arp::TaskCatalog(empty).tasks().empty(),
            "Catalogo vacio"
        );

        std::cout
            << "OK: IDs, orientaciones, opcionales, "
            << "paralelas y errores\n";

        return 0;

    } catch (const std::exception& error) {
        std::cerr << "FALLO: " << error.what() << '\n';
        return 1;
    }
}