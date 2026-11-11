#include "arp_routing.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int integer(const std::string& text) {
    if (text.empty()
        || text.find_first_not_of("0123456789")
            != std::string::npos) {

        throw std::runtime_error(
            "Se esperaba un entero no negativo: " + text
        );
    }

    return std::stoi(text);
}

double number(const std::string& text) {
    std::size_t used = 0;

    const double value = std::stod(text, &used);

    if (used != text.size()
        || !std::isfinite(value)
        || value < 0) {

        throw std::runtime_error(
            "Se esperaba un numero finito no negativo: "
            + text
        );
    }

    return value;
}

void usage() {
    std::cout
        << "Uso de caminos:\n"
        << "  arp_routes INSTANCIA --path ORIGEN DESTINO\n"
        << "\nUso de rutas:\n"
        << "  arp_routes INSTANCIA --capacity Q --demand-cost\n"
        << "\nAlternativa de carga uniforme:\n"
        << "  arp_routes INSTANCIA --capacity Q --demand q\n"
        << "\nOpciones adicionales:\n"
        << "  [--task-demand ID:q]... "
        << "[--start N] [--end N] [--details]\n"
        << "\n--demand-cost usa el costo de cada servicio como carga.\n"
        << "--demand usa una carga uniforme.\n";
}

void visits(const std::vector<arp::TaskVisit>& tasks) {
    for (const auto& task : tasks) {
        std::cout
            << ' '
            << task.taskId
            << (task.reversed ? '-' : '+');
    }

    std::cout << '\n';
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return 2;
    }

    if (std::string(argv[1]) == "--help") {
        usage();
        return 0;
    }

    try {
        double capacity = -1;
        double demand = -1;

        int start = -1;
        int end = -1;
        int from = -1;
        int to = -1;

        bool details = false;
        bool demandCost = false;

        std::map<int, double> overrides;

        for (int i = 2; i < argc; ++i) {
            const std::string option = argv[i];

            const auto next = [&]() -> std::string {
                if (++i >= argc) {
                    throw std::runtime_error(
                        "Falta valor para " + option
                    );
                }

                return argv[i];
            };

            if (option == "--capacity" && capacity < 0) {
                capacity = number(next());

            } else if (option == "--demand" && demand < 0) {
                demand = number(next());

            } else if (option == "--start" && start < 0) {
                start = integer(next());

            } else if (option == "--end" && end < 0) {
                end = integer(next());

            } else if (option == "--details" && !details) {
                details = true;

            } else if (option == "--demand-cost" && !demandCost) {
                demandCost = true;

            } else if (option == "--path" && from < 0) {
                from = integer(next());
                to = integer(next());

            } else if (option == "--task-demand") {
                const std::string value = next();
                const auto colon = value.find(':');

                if (colon == std::string::npos) {
                    throw std::runtime_error(
                        "Usa --task-demand ID:q"
                    );
                }

                const int id = integer(
                    value.substr(0, colon)
                );

                const double quantity = number(
                    value.substr(colon + 1)
                );

                if (!overrides.emplace(id, quantity).second) {
                    throw std::runtime_error(
                        "Demanda repetida para ID "
                        + std::to_string(id)
                    );
                }

            } else {
                throw std::runtime_error(
                    "Opcion desconocida o repetida: "
                    + option
                );
            }
        }

        arp::ArpInstance instance;
        instance.readDataFromFile(argv[1]);

        arp::ShortestPaths paths(instance);

        std::cout
            << std::setprecision(12)
            << "Nombre: " << instance.name << '\n';

        for (const auto& warning : instance.warnings) {
            std::cout
                << "AVISO: "
                << warning
                << '\n';
        }

        // Consulta independiente de un camino mínimo.
        if (from >= 0) {
            if (capacity >= 0
                || demand >= 0
                || demandCost
                || start >= 0
                || end >= 0
                || !overrides.empty()) {

                throw std::runtime_error(
                    "Consulta --path por separado "
                    "de los parametros de rutas"
                );
            }

            if (instance.declaredRestrictions
                || !instance.restrictionRows.empty()) {

                throw std::runtime_error(
                    "No se interpretan restricciones adicionales"
                );
            }

            const auto path = paths.path(from, to);

            if (!path.reachable) {
                std::cout << "SIN CAMINO\n";
                return 3;
            }

            std::cout
                << "Costo minimo: "
                << path.cost
                << "\nNodos:";

            for (int node : path.nodeIds) {
                std::cout << ' ' << node;
            }

            std::cout << "\nCalles:";

            for (int id : path.linkIds) {
                std::cout << ' ' << id;
            }

            std::cout << '\n';

            return 0;
        }

        // Para construir rutas se necesita capacidad y
        // una de las dos reglas de carga.
        if (capacity < 0
            || (!demandCost && demand < 0)
            || (demandCost && demand >= 0)) {

            throw std::runtime_error(
                "Indica --capacity Q y elige "
                "--demand-cost o --demand q"
            );
        }

        const std::size_t particularDemands =
            overrides.size();

        if (demandCost) {
            const arp::TaskCatalog catalog(instance);

            // Regla acordada:
            // carga del servicio = costo de su calle.
            for (const auto& task : catalog.tasks()) {
                overrides.emplace(
                    task.id,
                    task.cost
                );
            }
        }

        const auto problem = arp::makeProblem(
            instance,
            capacity,
            demandCost ? 0 : demand,
            overrides,
            start,
            end
        );

        const auto tour = arp::buildGiantTour(
            instance,
            problem,
            paths
        );

        const auto solution = arp::splitByCapacity(
            instance,
            problem,
            tour,
            paths
        );

        const auto validation = arp::validateSolution(
            instance,
            problem,
            solution
        );

        std::cout
            << "Inicio: " << problem.startNode
            << "\nTermino: " << problem.endNode
            << "\nCapacidad Q: " << problem.capacity
            << "\nCarga por tarea: ";

        if (demandCost) {
            std::cout << "costo original del servicio";
        } else {
            std::cout << demand;
        }

        std::cout
            << "\nExcepciones de carga indicadas: "
            << particularDemands
            << "\nGiant tour (+ directa, - inversa):";

        visits(tour);

        for (std::size_t r = 0;
             r < solution.routes.size();
             ++r) {

            const auto& route = solution.routes[r];

            std::cout
                << "Ruta " << r + 1
                << ": carga=" << route.load
                << '/' << problem.capacity
                << " costo=" << route.cost
                << " tareas:";

            visits(route.tasks);

            if (details) {
                for (const auto& move : route.walk) {
                    std::cout
                        << "  "
                        << (move.service
                            ? "servicio"
                            : "transito")
                        << " ID " << move.linkId
                        << ": " << move.from
                        << " -> " << move.to
                        << '\n';
                }
            }
        }

        std::cout
            << "Rutas: " << solution.routes.size()
            << "\nCosto total: " << solution.cost
            << "\nValidacion: "
            << (validation.valid()
                ? "VALIDA"
                : "INVALIDA")
            << '\n';

        for (const auto& error : validation.errors) {
            std::cerr
                << "ERROR: "
                << error
                << '\n';
        }

        return validation.valid() ? 0 : 3;

    } catch (const std::exception& error) {
        std::cerr
            << "ERROR: "
            << error.what()
            << '\n';

        return 1;
    }
}
