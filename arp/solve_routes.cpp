#include "arp_decoder.h"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

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
        << "  arp_routes INSTANCIA --capacity Q [--demand-file]\n"

        << "\nAlternativa de carga uniforme:\n"
        << "  arp_routes INSTANCIA --capacity Q --demand q\n"

        << "\nOpciones adicionales:\n"
        << "  [--task-demand ID:q]... "
        << "[--start N] [--end N] [--details]\n"

        << "\nPor defecto, la carga se lee de la quinta columna.\n"
        << "La cuarta columna conserva el tiempo; "
        << "la quinta indica carga.\n"
        << "--demand usa una carga uniforme.\n"

        << "[--decoder first-traversal|scheduled]\n"
        << "Por defecto: first-traversal "
        << "(procedimiento de primera travesia).\n";
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

// Vista enlazada del orden real de servicios.
// Esta estructura se construye para mostrar el resultado.
void printLinkedLists(
    const arp::RoutingSolution& solution,
    const arp::RoutingProblem& problem,
    bool details
) {
    const std::size_t none =
        std::numeric_limits<std::size_t>::max();

    struct Entry {
        const arp::TaskVisit* visit;
        std::size_t previous;
        std::size_t next;
    };

    struct Ends {
        std::size_t first;
        std::size_t last;
    };

    std::vector<Entry> entries;
    std::vector<Ends> routes;

    entries.reserve(solution.giantTour.size());
    routes.reserve(solution.routes.size());

    for (const auto& route : solution.routes) {
        Ends ends{none, none};

        for (const auto& visit : route.tasks) {
            const std::size_t index = entries.size();

            entries.push_back({
                &visit,
                ends.last,
                none
            });

            if (ends.last == none) {
                ends.first = index;
            } else {
                entries[ends.last].next = index;
            }

            ends.last = index;
        }

        routes.push_back(ends);
    }

    std::cout
        << "\nREPRESENTACION ADICIONAL: "
        << "LISTAS DOBLEMENTE ENLAZADAS\n"
        << "Cada elemento representa un servicio requerido.\n"
        << "Los enlaces indican orden en la lista, "
        << "no sentido vial.\n";

    for (std::size_t r = 0; r < routes.size(); ++r) {
        const auto& route = solution.routes[r];

        std::cout
            << "Lista de ruta " << r + 1
            << ": INICIO(" << problem.startNode << ')';

        for (std::size_t i = routes[r].first;
             i != none;
             i = entries[i].next) {
            const auto& visit = *entries[i].visit;

            std::cout
                << " <-> " << visit.taskId
                << (visit.reversed ? '-' : '+');
        }

        std::cout
            << " <-> FIN(" << problem.endNode << ')'
            << " | carga=" << route.load
            << '/' << problem.capacity
            << " | tiempo=" << route.cost
            << '\n';

        if (details) {
            std::cout
                << "  POSICION ID ORIENTACION ANTERIOR SIGUIENTE\n";

            for (std::size_t i = routes[r].first;
                 i != none;
                 i = entries[i].next) {
                const auto& entry = entries[i];

                std::cout
                    << "  " << i + 1
                    << ' ' << entry.visit->taskId
                    << ' '
                    << (entry.visit->reversed ? '-' : '+')
                    << ' ';

                if (entry.previous == none) {
                    std::cout << "INICIO";
                } else {
                    std::cout << entry.previous + 1;
                }

                std::cout << ' ';

                if (entry.next == none) {
                    std::cout << "FIN";
                } else {
                    std::cout << entry.next + 1;
                }

                std::cout << '\n';
            }
        }
    }
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
        bool demandFile = false;
        bool decoderSet = false;

        std::string decoder = "first-traversal";

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

            if (option == "--decoder" && !decoderSet) {
                decoder = next();
                decoderSet = true;

                if (decoder != "first-traversal"
                    && decoder != "scheduled") {
                    throw std::runtime_error(
                        "Decodificador desconocido: " + decoder
                    );
                }

            } else if (option == "--capacity" && capacity < 0) {
                capacity = number(next());

            } else if (option == "--demand" && demand < 0) {
                demand = number(next());

            } else if (option == "--start" && start < 0) {
                start = integer(next());

            } else if (option == "--end" && end < 0) {
                end = integer(next());

            } else if (option == "--details" && !details) {
                details = true;

            } else if (option == "--demand-file" && !demandFile) {
                demandFile = true;

            } else if (option == "--demand-cost") {
                throw std::runtime_error(
                    "La cuarta columna es tiempo, no carga. "
                    "Elimina --demand-cost y usa --demand-file."
                );

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
                    "Opcion desconocida o repetida: " + option
                );
            }
        }

        arp::ArpInstance instance;
        instance.readDataFromFile(argv[1]);

        arp::ShortestPaths paths(instance);

        std::cout
            << std::setprecision(12)
            << "Nombre: " << instance.name
            << '\n';

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
                || demandFile
                || decoderSet
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

            std::cout
                << "Dijkstra: ejecuciones="
                << paths.dijkstraCalls()
                << " | tiempo acumulado (ms)="
                << paths.dijkstraMilliseconds()
                << '\n';

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

        if (capacity < 0 || (demandFile && demand >= 0)) {
            throw std::runtime_error(
                "Indica --capacity Q; "
                "--demand-file y --demand q son excluyentes"
            );
        }

        const std::size_t particularDemands =
            overrides.size();

        const bool useFileDemand = demand < 0;

        const auto problem = useFileDemand
            ? arp::makeProblemFromInstance(
                instance,
                capacity,
                overrides,
                start,
                end
            )
            : arp::makeProblem(
                instance,
                capacity,
                demand,
                overrides,
                start,
                end
            );

        using Clock = std::chrono::steady_clock;
        using Milliseconds =
            std::chrono::duration<double, std::milli>;

        // Estadísticas de Dijkstra antes del constructor.
        const auto callsBeforeGiant =
            paths.dijkstraCalls();

        const double msBeforeGiant =
            paths.dijkstraMilliseconds();

        // Tiempo de construcción del Giant Tour.
        const auto giantStart = Clock::now();

        const auto tour = arp::buildGiantTour(
            instance,
            problem,
            paths
        );

        const double giantMs =
            Milliseconds(Clock::now() - giantStart).count();

        const auto giantDijkstraCalls =
            paths.dijkstraCalls() - callsBeforeGiant;

        const double giantDijkstraMs =
            paths.dijkstraMilliseconds() - msBeforeGiant;

        arp::RoutingSolution solution;
        arp::ValidationResult validation;

        bool complete = true;
        std::string failure;

        double decodingMs = 0;
        double validationMs = 0;

        const auto callsBeforeDecoding =
            paths.dijkstraCalls();

        const double msBeforeDecoding =
            paths.dijkstraMilliseconds();

        if (decoder == "first-traversal") {
            // Incluye construcción de recorridos y objetivo.
            const auto decodingStart = Clock::now();

            auto decoded = arp::decodeFirstTraversal(
                instance,
                problem,
                tour,
                paths
            );

            decodingMs =
                Milliseconds(
                    Clock::now() - decodingStart
                ).count();

            const auto validationStart = Clock::now();

            validation = arp::validateFirstTraversal(
                instance,
                problem,
                decoded
            );

            validationMs =
                Milliseconds(
                    Clock::now() - validationStart
                ).count();

            complete = decoded.complete;
            failure = decoded.failure;

            solution.giantTour = tour;
            solution.cost = decoded.cost;
            solution.routes = std::move(decoded.routes);

        } else {
            const auto decodingStart = Clock::now();

            solution = arp::splitByCapacity(
                instance,
                problem,
                tour,
                paths
            );

            decodingMs =
                Milliseconds(
                    Clock::now() - decodingStart
                ).count();

            const auto validationStart = Clock::now();

            validation = arp::validateSolution(
                instance,
                problem,
                solution
            );

            validationMs =
                Milliseconds(
                    Clock::now() - validationStart
                ).count();
        }

        const double evaluationMs =
            decodingMs + validationMs;

        const double measuredPhasesMs =
            giantMs + evaluationMs;

        const auto decodingDijkstraCalls =
            paths.dijkstraCalls() - callsBeforeDecoding;

        const double decodingDijkstraMs =
            paths.dijkstraMilliseconds() - msBeforeDecoding;

        std::cout
            << "Decodificador: "
            << decoder
            << '\n';

        if (!complete) {
            std::cout
                << "Estado: INCOMPLETA\n"
                << "Motivo: " << failure
                << "\nLas rutas mostradas son parciales.\n";
        }

        std::cout
            << "Inicio: " << problem.startNode
            << "\nTermino: " << problem.endNode
            << "\nCapacidad Q: " << problem.capacity
            << "\nCarga por tarea: ";

        if (useFileDemand) {
            std::cout << "quinta columna de la instancia";
        } else {
            std::cout << demand;
        }

        std::cout
            << "\nExcepciones de carga indicadas: "
            << particularDemands
            << (
                decoder == "first-traversal"
                    ? "\nGiant tour de prioridad "
                      "(+/- orientacion preferida):"
                    : "\nGiant tour (+ directa, - inversa):"
            );

        visits(tour);

        if (decoder == "first-traversal") {
            std::cout << "Orden real de recoleccion:";

            for (const auto& route : solution.routes) {
                for (const auto& visit : route.tasks) {
                    std::cout
                        << ' '
                        << visit.taskId
                        << (visit.reversed ? '-' : '+');
                }
            }

            std::cout << '\n';
        }

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

        printLinkedLists(
            solution,
            problem,
            details
        );

        // Estos tiempos excluyen lectura e impresión.
        std::cout
            << "\nTiempos computacionales (ms):\n"
            << "  Construccion giant tour: "
            << giantMs

            << "\n  Decodificacion + funcion objetivo: "
            << decodingMs

            << "\n  Validacion: "
            << validationMs

            << "\n  Evaluacion (decodificacion + validacion): "
            << evaluationMs

            << "\n  Suma de las fases medidas: "
            << measuredPhasesMs

            << "\nLectura e impresion excluidas de estas mediciones.\n";

        const auto totalDijkstraCalls =
            paths.dijkstraCalls();

        const double totalDijkstraMs =
            paths.dijkstraMilliseconds();

        const double averageDijkstraMs =
            totalDijkstraCalls > 0
                ? totalDijkstraMs / totalDijkstraCalls
                : 0.0;

        // Desglose: Dijkstra ya está incluido en las fases anteriores.
        std::cout
            << "\nTiempos de Dijkstra (ms):\n"
            << "  Giant tour: "
            << giantDijkstraMs
            << " | ejecuciones: "
            << giantDijkstraCalls

            << "\n  Decodificacion: "
            << decodingDijkstraMs
            << " | ejecuciones: "
            << decodingDijkstraCalls

            << "\n  Total acumulado: "
            << totalDijkstraMs

            << "\n  Ejecuciones reales: "
            << totalDijkstraCalls

            << "\n  Promedio por ejecucion: "
            << averageDijkstraMs
            << '\n';

        return validation.valid() ? 0 : 3;

    } catch (const std::exception& error) {
        std::cerr
            << "ERROR: "
            << error.what()
            << '\n';

        return 1;
    }
}