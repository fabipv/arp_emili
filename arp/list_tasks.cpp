#include "arp_tasks.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3
        || (argc == 3 && std::string(argv[2]) != "--all")) {

        std::cerr << "Uso: arp_tasks RUTA_INSTANCIA [--all]\n";
        return 2;
    }

    try {
        arp::ArpInstance instance;
        instance.readDataFromFile(argv[1]);

        // Conserva el catálogo de servicios obligatorios.
        const arp::TaskCatalog catalog(instance);

        std::size_t bidirectional = 0;

        for (const auto& task : catalog.tasks()) {
            if (task.reversible) {
                ++bidirectional;
            }
        }

        const std::size_t totalTasks = catalog.tasks().size();
        const std::size_t totalLinks = instance.links.size();

        std::cout << std::setprecision(12);

        std::cout
            << "Nombre: " << instance.name
            << "\nCalles totales: " << totalLinks
            << "\nTareas requeridas: " << totalTasks
            << "\nTareas uni: " << totalTasks - bidirectional
            << "\nTareas bi: " << bidirectional
            << "\nOrientaciones de servicio permitidas: "
            << totalTasks + bidirectional
            << "\nCalles no requeridas (transito): "
            << totalLinks - totalTasks
            << '\n';

        for (const auto& warning : instance.warnings) {
            std::cout << "AVISO: " << warning << '\n';
        }

        // Ahora se muestran todas las clases de calles.
        // Sin --all se muestran únicamente las primeras cinco.
        const std::size_t shown =
            argc == 3
                ? totalLinks
                : std::min<std::size_t>(5, totalLinks);

        std::cout
            << "Catalogo de calles: " << shown
            << " de " << totalLinks
            << " calles\n";

        // Recorre todas las calles de la instancia:
        // tanto requeridas como no requeridas.
        for (std::size_t i = 0; i < shown; ++i) {
            const auto& link = instance.links[i];

            std::cout
                << "ID " << link.id
                << " | "
                << (link.required ? "requerida" : "no requerida")
                << " | "
                << (link.directed ? "uni" : "bi")
                << " | directa "
                << link.from << " -> " << link.to;

            // Una calle bi permite circular en ambos sentidos.
            if (!link.directed) {
                std::cout
                    << " | inversa "
                    << link.to << " -> " << link.from;
            }

            std::cout
                << " | tiempo " << link.cost
                << " | carga " << link.demand
                << " | linea " << link.sourceLine
                << '\n';
        }

        return 0;

    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
}
